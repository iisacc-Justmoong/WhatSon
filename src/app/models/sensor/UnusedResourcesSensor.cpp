#include "app/models/sensor/UnusedResourcesSensor.hpp"

#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QVariantMap>

#include <utility>

namespace
{
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
