#include "UnusedResourcesSensor.hpp"

#include "models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "models/file/hub/WhatSonHubPathUtils.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantMap>

#include <utility>

namespace
{
    QString decodeXmlEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    QString extractXmlAttributeValue(const QString& tagText, const QStringList& attributeNames)
    {
        for (const QString& attributeName : attributeNames)
        {
            const QRegularExpression attributePattern(
                QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+?)(?=\s|/?>)))ATTR")
                    .arg(QRegularExpression::escape(attributeName)),
                QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match = attributePattern.match(tagText);
            if (!match.hasMatch())
            {
                continue;
            }

            for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
            {
                if (match.capturedStart(captureIndex) < 0)
                {
                    continue;
                }

                const QString value = decodeXmlEntities(match.captured(captureIndex)).trimmed();
                if (!value.isEmpty())
                {
                    return value;
                }
            }
        }

        return {};
    }

    QString validatedHubPath(const QString& hubPath, QString* errorMessage)
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(hubPath);
        const QFileInfo hubInfo(normalizedHubPath);
        if (!hubInfo.exists() || !hubInfo.isDir()
            || !hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub path is not an unpacked .wshub directory: %1").arg(
                    normalizedHubPath);
            }
            return {};
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return normalizedHubPath;
    }

    bool isSystemHubRootSegment(const QString& segment)
    {
        const QString folded = segment.trimmed().toCaseFolded();
        return folded.endsWith(QStringLiteral(".wscontents"))
            || folded.endsWith(QStringLiteral(".wsresources"));
    }

    bool shouldIgnoreHubPath(const QString& absolutePath, const QString& hubRootPath)
    {
        if (absolutePath.trimmed().isEmpty() || hubRootPath.trimmed().isEmpty())
        {
            return false;
        }

        const QString relativePath = QDir(hubRootPath).relativeFilePath(absolutePath);
        const QStringList segments = relativePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        for (int segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex)
        {
            const QString segment = segments.at(segmentIndex).trimmed();
            if (!segment.startsWith(QLatin1Char('.')))
            {
                continue;
            }
            if (segmentIndex == 0 && isSystemHubRootSegment(segment))
            {
                continue;
            }
            return true;
        }

        return false;
    }

    QStringList extractEmbeddedResourcePaths(const QString& bodyDocumentText)
    {
        static const QRegularExpression resourcePattern(
            QStringLiteral(R"(<\s*resource\b[^>]*?/?>)"),
            QRegularExpression::CaseInsensitiveOption);

        QStringList resourcePaths;
        QRegularExpressionMatchIterator iterator = resourcePattern.globalMatch(bodyDocumentText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            const QString resourcePath = WhatSon::Resources::normalizePath(
                extractXmlAttributeValue(
                    match.captured(0),
                    {
                        QStringLiteral("resourcePath"),
                        QStringLiteral("path"),
                        QStringLiteral("src"),
                        QStringLiteral("href"),
                        QStringLiteral("url")
                    }));
            if (resourcePath.isEmpty())
            {
                continue;
            }

            const QFileInfo referencedInfo(resourcePath);
            if (!WhatSon::Resources::isResourcePackageDirectoryName(referencedInfo.fileName()))
            {
                continue;
            }

            if (!resourcePaths.contains(resourcePath))
            {
                resourcePaths.push_back(resourcePath);
            }
        }

        return resourcePaths;
    }

    QSet<QString> collectEmbeddedResourceLookup(const QString& normalizedHubPath, QString* errorMessage)
    {
        QSet<QString> embeddedResourceLookup;

        QDirIterator iterator(
            normalizedHubPath,
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden,
            QDirIterator::Subdirectories);
        while (iterator.hasNext())
        {
            const QString currentPath = WhatSon::Resources::normalizePath(iterator.next());
            const QFileInfo currentInfo(currentPath);
            if (!currentInfo.isFile() || currentInfo.suffix().compare(QStringLiteral("wsnbody"), Qt::CaseInsensitive) != 0)
            {
                continue;
            }
            if (shouldIgnoreHubPath(currentPath, normalizedHubPath))
            {
                continue;
            }

            QFile bodyFile(currentPath);
            if (!bodyFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to open note body file: %1").arg(currentPath);
                }
                return {};
            }

            const QString bodyDocumentText = QString::fromUtf8(bodyFile.readAll());
            const QStringList referencedPaths = extractEmbeddedResourcePaths(bodyDocumentText);
            for (const QString& referencedPath : referencedPaths)
            {
                embeddedResourceLookup.insert(referencedPath.toCaseFolded());
            }
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return embeddedResourceLookup;
    }

    QString fallbackAssetFilePathForPackage(const QString& packageDirectoryPath)
    {
        const QFileInfoList candidateFiles = QDir(packageDirectoryPath).entryInfoList(
            QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QFileInfo& candidateFile : candidateFiles)
        {
            const QString fileName = candidateFile.fileName().trimmed();
            if (fileName.compare(WhatSon::Resources::metadataFileName(), Qt::CaseInsensitive) == 0
                || fileName.compare(WhatSon::Resources::annotationFileName(), Qt::CaseInsensitive) == 0)
            {
                continue;
            }
            return candidateFile.absoluteFilePath();
        }

        return {};
    }

    QVariantMap buildUnusedResourceEntry(
        const QString& packageDirectoryPath,
        const QString& resourcePath,
        WhatSon::Resources::ResourcePackageMetadata metadata,
        const bool metadataValid,
        QString metadataError)
    {
        const QString normalizedPackageDirectoryPath = WhatSon::Resources::normalizePath(packageDirectoryPath);
        const QString normalizedResourcePath = WhatSon::Resources::normalizePath(resourcePath);

        const QString resolvedAssetFilePath = metadataValid
            ? WhatSon::Resources::resolveAssetPathFromMetadata(normalizedPackageDirectoryPath, metadata)
            : fallbackAssetFilePathForPackage(normalizedPackageDirectoryPath);

        if (!metadataValid)
        {
            metadata = WhatSon::Resources::buildFallbackMetadataFromResourcePath(
                normalizedResourcePath,
                resolvedAssetFilePath);
        }

        QVariantMap entry;
        entry.insert(QStringLiteral("resourcePath"), normalizedResourcePath);
        entry.insert(QStringLiteral("packageDirectoryPath"), normalizedPackageDirectoryPath);
        entry.insert(QStringLiteral("packageName"), QFileInfo(normalizedPackageDirectoryPath).fileName().trimmed());
        entry.insert(QStringLiteral("resourceId"), metadata.resourceId.trimmed());
        entry.insert(QStringLiteral("assetPath"), metadata.assetPath.trimmed());
        entry.insert(QStringLiteral("assetAbsolutePath"), WhatSon::Resources::normalizePath(resolvedAssetFilePath));
        entry.insert(QStringLiteral("annotationPath"), metadata.annotationPath.trimmed());
        entry.insert(
            QStringLiteral("annotationAbsolutePath"),
            WhatSon::Resources::normalizePath(
                WhatSon::Resources::annotationFilePathForPackage(normalizedPackageDirectoryPath)));
        entry.insert(QStringLiteral("bucket"), metadata.bucket.trimmed());
        entry.insert(QStringLiteral("type"), metadata.type.trimmed());
        entry.insert(QStringLiteral("format"), metadata.format.trimmed());
        entry.insert(QStringLiteral("metadataValid"), metadataValid);
        entry.insert(QStringLiteral("metadataError"), std::move(metadataError));
        return entry;
    }

    QVariantList collectUnusedResourceEntries(const QString& normalizedHubPath, QString* errorMessage)
    {
        QVariantList unusedResources;

        const QStringList resourceRoots = WhatSon::Resources::resolveResourceRootDirectories(normalizedHubPath);
        if (resourceRoots.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("No *.wsresources directory found inside hub: %1").arg(
                    normalizedHubPath);
            }
            return unusedResources;
        }

        QString embeddedLookupError;
        const QSet<QString> embeddedResourceLookup = collectEmbeddedResourceLookup(
            normalizedHubPath,
            &embeddedLookupError);
        if (!embeddedLookupError.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = embeddedLookupError;
            }
            return {};
        }

        for (const QString& resourceRoot : resourceRoots)
        {
            const QFileInfoList packageDirectories = QDir(resourceRoot).entryInfoList(
                QStringList{QStringLiteral("*.wsresource")},
                QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::Name);
            for (const QFileInfo& packageDirectory : packageDirectories)
            {
                const QString packageDirectoryPath = WhatSon::Resources::normalizePath(
                    packageDirectory.absoluteFilePath());
                if (shouldIgnoreHubPath(packageDirectoryPath, normalizedHubPath))
                {
                    continue;
                }

                const QString resourcePath =
                    WhatSon::Resources::resourcePathForPackageDirectory(packageDirectoryPath);
                if (embeddedResourceLookup.contains(resourcePath.toCaseFolded()))
                {
                    continue;
                }

                WhatSon::Resources::ResourcePackageMetadata metadata;
                QString metadataError;
                const bool metadataValid = WhatSon::Resources::loadResourcePackageMetadata(
                    packageDirectoryPath,
                    &metadata,
                    &metadataError);
                unusedResources.push_back(buildUnusedResourceEntry(
                    packageDirectoryPath,
                    resourcePath,
                    metadata,
                    metadataValid,
                    metadataError));
            }
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return unusedResources;
    }
} // namespace

UnusedResourcesSensor::UnusedResourcesSensor(QObject* parent)
    : QObject(parent)
{
}

UnusedResourcesSensor::~UnusedResourcesSensor() = default;

QString UnusedResourcesSensor::hubPath() const
{
    return m_hubPath;
}

void UnusedResourcesSensor::setHubPath(QString hubPath)
{
    const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (m_hubPath == normalizedHubPath)
    {
        return;
    }

    m_hubPath = std::move(normalizedHubPath);
    emit hubPathChanged();
    refresh();
}

QVariantList UnusedResourcesSensor::unusedResources() const
{
    return m_unusedResources;
}

QStringList UnusedResourcesSensor::unusedResourcePaths() const
{
    QStringList values;
    values.reserve(m_unusedResources.size());
    for (const QVariant& entryValue : m_unusedResources)
    {
        const QString resourcePath = entryValue.toMap().value(QStringLiteral("resourcePath")).toString().trimmed();
        if (!resourcePath.isEmpty())
        {
            values.push_back(resourcePath);
        }
    }
    return values;
}

int UnusedResourcesSensor::unusedResourceCount() const noexcept
{
    return m_unusedResources.size();
}

QString UnusedResourcesSensor::lastError() const
{
    return m_lastError;
}

QVariantList UnusedResourcesSensor::scanUnusedResources(const QString& hubPath)
{
    const QString trimmedHubPath = hubPath.trimmed();
    if (!trimmedHubPath.isEmpty())
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(trimmedHubPath);
        if (m_hubPath != normalizedHubPath)
        {
            m_hubPath = normalizedHubPath;
            emit hubPathChanged();
        }
        refresh();
        return m_unusedResources;
    }

    refresh();
    return m_unusedResources;
}

QStringList UnusedResourcesSensor::collectUnusedResourcePaths(const QString& hubPath)
{
    return scanUnusedResources(hubPath).isEmpty() ? QStringList{} : unusedResourcePaths();
}

void UnusedResourcesSensor::refresh()
{
    if (m_hubPath.trimmed().isEmpty())
    {
        setLastError(QString());
        setUnusedResources({});
        emit scanCompleted(m_unusedResources);
        return;
    }

    QString validationError;
    const QString normalizedHubPath = validatedHubPath(m_hubPath, &validationError);
    if (normalizedHubPath.isEmpty())
    {
        setLastError(validationError);
        setUnusedResources({});
        emit scanCompleted(m_unusedResources);
        return;
    }

    QString scanError;
    const QVariantList nextUnusedResources = collectUnusedResourceEntries(normalizedHubPath, &scanError);
    setLastError(scanError);
    setUnusedResources(nextUnusedResources);
    emit scanCompleted(m_unusedResources);
}

void UnusedResourcesSensor::setLastError(QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    if (m_lastError == errorMessage)
    {
        return;
    }

    m_lastError = std::move(errorMessage);
    emit lastErrorChanged();
}

void UnusedResourcesSensor::setUnusedResources(QVariantList unusedResources)
{
    if (m_unusedResources == unusedResources)
    {
        return;
    }

    m_unusedResources = std::move(unusedResources);
    emit unusedResourcesChanged();
}
