#include "ResourcesImportViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMetaType>
#include <QMimeData>
#include <QPixmap>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSequentialIterable>
#include <QSet>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>

#include <utility>

namespace
{
    constexpr auto kScope = "resources.import";

    QString resolvePrimaryHubDirectory(
        const QString& hubPath,
        const QString& fixedDirectoryName,
        const QString& wildcardPattern,
        QString* errorMessage = nullptr)
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(hubPath);
        const QFileInfo hubInfo(normalizedHubPath);
        if (!hubInfo.exists() || !hubInfo.isDir()
            || !hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Current hub path is not an unpacked .wshub directory: %1").arg(
                    normalizedHubPath);
            }
            return {};
        }

        const QDir hubDir(normalizedHubPath);
        const QString fixedPath = hubDir.filePath(fixedDirectoryName);
        if (QFileInfo(fixedPath).isDir())
        {
            return WhatSon::Resources::normalizePath(fixedPath);
        }

        const QStringList candidates = hubDir.entryList(
            QStringList{wildcardPattern},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        if (!candidates.isEmpty())
        {
            return WhatSon::Resources::normalizePath(hubDir.filePath(candidates.constFirst()));
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Required hub directory is missing: %1 inside %2").arg(
                wildcardPattern,
                normalizedHubPath);
        }
        return {};
    }

    QString resolveContentsDirectory(const QString& hubPath, QString* errorMessage = nullptr)
    {
        return resolvePrimaryHubDirectory(
            hubPath,
            QStringLiteral(".wscontents"),
            QStringLiteral("*.wscontents"),
            errorMessage);
    }

    QString resourceRootNameFromResourcePath(const QString& resourcePath)
    {
        const QString normalizedPath = WhatSon::Resources::normalizePath(resourcePath.trimmed());
        const int separatorIndex = normalizedPath.indexOf(QLatin1Char('/'));
        if (separatorIndex <= 0)
        {
            return {};
        }

        const QString rootName = normalizedPath.left(separatorIndex).trimmed();
        if (!rootName.endsWith(QStringLiteral(".wsresources"), Qt::CaseInsensitive))
        {
            return {};
        }
        return rootName;
    }

    QString resolveResourcesDirectory(
        const QString& hubPath,
        const QStringList& existingResourcePaths = {},
        QString* errorMessage = nullptr)
    {
        const QStringList candidates = WhatSon::Resources::resolveResourceRootDirectories(hubPath);
        if (candidates.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Required hub directory is missing: *.wsresources inside %1").arg(
                    WhatSon::HubPath::normalizeAbsolutePath(hubPath));
            }
            return {};
        }

        QSet<QString> referencedRootNames;
        for (const QString& resourcePath : existingResourcePaths)
        {
            const QString rootName = resourceRootNameFromResourcePath(resourcePath);
            if (rootName.isEmpty())
            {
                continue;
            }
            referencedRootNames.insert(rootName.toCaseFolded());
        }

        if (!referencedRootNames.isEmpty())
        {
            for (const QString& candidatePath : candidates)
            {
                const QString candidateDirectoryName = QFileInfo(candidatePath).fileName().trimmed().toCaseFolded();
                if (referencedRootNames.contains(candidateDirectoryName))
                {
                    return WhatSon::Resources::normalizePath(candidatePath);
                }
            }
        }

        return candidates.constFirst();
    }

    QString sanitizeResourceId(QString value)
    {
        value = QFileInfo(value.trimmed()).completeBaseName().trimmed().toCaseFolded();
        value.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]+")), QStringLiteral("-"));
        value.replace(QRegularExpression(QStringLiteral("-{2,}")), QStringLiteral("-"));
        value.remove(QRegularExpression(QStringLiteral("^-+")));
        value.remove(QRegularExpression(QStringLiteral("-+$")));
        if (value.isEmpty())
        {
            value = QStringLiteral("resource");
        }
        return value;
    }

    QString uniqueResourceIdForFile(const QString& resourcesDirectoryPath, const QString& sourceFilePath)
    {
        const QFileInfoList packageDirectories = QDir(resourcesDirectoryPath).entryInfoList(
            QStringList{QStringLiteral("*.wsresource")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);

        QSet<QString> existingIds;
        existingIds.reserve(packageDirectories.size());
        for (const QFileInfo& packageDirectory : packageDirectories)
        {
            existingIds.insert(
                WhatSon::Resources::resourceIdFromPackageName(packageDirectory.fileName()).toCaseFolded());
        }

        const QString baseId = sanitizeResourceId(sourceFilePath);
        QString candidateId = baseId;
        int suffix = 2;
        while (existingIds.contains(candidateId.toCaseFolded()))
        {
            candidateId = QStringLiteral("%1-%2").arg(baseId).arg(suffix);
            ++suffix;
        }

        return candidateId;
    }

    bool writeUtf8FileAtomically(const QString& filePath, const QString& text, QString* errorMessage = nullptr)
    {
        const QString directoryPath = QFileInfo(filePath).absolutePath();
        if (!directoryPath.isEmpty() && !QDir().mkpath(directoryPath))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to create directory for file write: %1").arg(directoryPath);
            }
            return false;
        }

        QSaveFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file for write: %1").arg(filePath);
            }
            return false;
        }

        if (file.write(text.toUtf8()) < 0)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write file: %1").arg(filePath);
            }
            file.cancelWriting();
            return false;
        }

        if (!file.commit())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to commit file write: %1").arg(filePath);
            }
            return false;
        }

        return true;
    }

    bool readUtf8FileText(const QString& filePath, QString* outText, QString* errorMessage = nullptr)
    {
        if (outText == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outText must not be null.");
            }
            return false;
        }

        outText->clear();
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file for read: %1").arg(filePath);
            }
            return false;
        }

        *outText = QString::fromUtf8(file.readAll());
        return true;
    }

    void appendLocalFilePath(const QString& filePath, QStringList* localFiles)
    {
        if (localFiles == nullptr)
        {
            return;
        }

        const QString absolutePath = WhatSon::HubPath::normalizeAbsolutePath(filePath);
        const QFileInfo fileInfo(absolutePath);
        if (!fileInfo.exists() || !fileInfo.isFile())
        {
            return;
        }

        if (!localFiles->contains(absolutePath))
        {
            localFiles->push_back(absolutePath);
        }
    }

    void appendLocalFilesFromVariant(const QVariant& entry, QStringList* localFiles)
    {
        if (localFiles == nullptr || !entry.isValid())
        {
            return;
        }

        if (entry.metaType().id() == QMetaType::QVariantList)
        {
            const QVariantList nestedValues = entry.toList();
            for (const QVariant& nestedValue : nestedValues)
            {
                appendLocalFilesFromVariant(nestedValue, localFiles);
            }
            return;
        }

        if (entry.metaType().id() == QMetaType::QStringList)
        {
            const QStringList nestedValues = entry.toStringList();
            for (const QString& nestedValue : nestedValues)
            {
                appendLocalFilesFromVariant(nestedValue, localFiles);
            }
            return;
        }

        if (entry.canConvert<QSequentialIterable>())
        {
            const QSequentialIterable iterable = entry.value<QSequentialIterable>();
            bool iteratedAny = false;
            for (auto it = iterable.begin(); it != iterable.end(); ++it)
            {
                iteratedAny = true;
                appendLocalFilesFromVariant(*it, localFiles);
            }
            if (iteratedAny)
            {
                return;
            }
        }

        QUrl url = entry.toUrl();
        if (!url.isValid() || url.isEmpty())
        {
            const QString rawText = entry.toString().trimmed();
            if (rawText.isEmpty())
            {
                return;
            }
            url = QUrl::fromUserInput(rawText);
        }

        if (!url.isValid() || !url.isLocalFile())
        {
            return;
        }

        appendLocalFilePath(url.toLocalFile(), localFiles);
    }

    QStringList extractDroppedLocalFiles(const QVariantList& urls)
    {
        QStringList localFiles;
        for (const QVariant& entry : urls)
        {
            appendLocalFilesFromVariant(entry, &localFiles);
        }
        return localFiles;
    }

    bool loadExistingResourcePaths(const QString& resourcesFilePath, QStringList* outPaths, QString* errorMessage = nullptr)
    {
        if (outPaths == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outPaths must not be null.");
            }
            return false;
        }

        outPaths->clear();
        if (!QFileInfo(resourcesFilePath).isFile())
        {
            return true;
        }

        QFile file(resourcesFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open Resources.wsresources: %1").arg(resourcesFilePath);
            }
            return false;
        }

        WhatSonResourcesHierarchyParser parser;
        WhatSonResourcesHierarchyStore store;
        QString parseError;
        const QString rawText = QString::fromUtf8(file.readAll());
        if (!parser.parse(rawText, &store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        *outPaths = store.resourcePaths();
        return true;
    }

    bool importSingleFile(
        const QString& sourceFilePath,
        const QString& resourcesDirectoryPath,
        QString* outResourcePath,
        QString* outCreatedPackagePath,
        WhatSon::Resources::ResourcePackageMetadata* outMetadata,
        QString* errorMessage = nullptr)
    {
        if (outResourcePath == nullptr || outCreatedPackagePath == nullptr || outMetadata == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Output pointers must not be null.");
            }
            return false;
        }

        *outResourcePath = QString();
        *outCreatedPackagePath = QString();
        *outMetadata = {};

        const QFileInfo sourceFileInfo(sourceFilePath);
        if (!sourceFileInfo.exists() || !sourceFileInfo.isFile())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Dropped file does not exist: %1").arg(sourceFilePath);
            }
            return false;
        }

        const QString resourceId = uniqueResourceIdForFile(resourcesDirectoryPath, sourceFilePath);
        const QString packageDirectoryPath = QDir(resourcesDirectoryPath).filePath(
            resourceId + WhatSon::Resources::packageDirectorySuffix());
        const QString destinationAssetPath = QDir(packageDirectoryPath).filePath(sourceFileInfo.fileName());
        const QString resourcePath = WhatSon::Resources::normalizePath(
            QStringLiteral("%1/%2")
                .arg(QFileInfo(resourcesDirectoryPath).fileName(), QFileInfo(packageDirectoryPath).fileName()));

        if (!QDir().mkpath(packageDirectoryPath))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to create resource package directory: %1").arg(
                    packageDirectoryPath);
            }
            return false;
        }

        if (!QFile::copy(sourceFilePath, destinationAssetPath))
        {
            QDir(packageDirectoryPath).removeRecursively();
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to copy dropped file into resource package: %1").arg(
                    sourceFilePath);
            }
            return false;
        }

        const WhatSon::Resources::ResourcePackageMetadata metadata =
            WhatSon::Resources::buildMetadataForAssetFile(
                sourceFileInfo.fileName(),
                resourceId,
                resourcePath);

        QString writeError;
        if (!writeUtf8FileAtomically(
            QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName()),
            WhatSon::Resources::createResourcePackageMetadataXml(metadata),
            &writeError))
        {
            QDir(packageDirectoryPath).removeRecursively();
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }

        *outResourcePath = resourcePath;
        *outCreatedPackagePath = packageDirectoryPath;
        *outMetadata = metadata;
        return true;
    }

    QVariantMap importedEntryFromMetadata(const WhatSon::Resources::ResourcePackageMetadata& metadata)
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("resourceId"), metadata.resourceId.trimmed());
        entry.insert(QStringLiteral("resourcePath"), WhatSon::Resources::normalizePath(metadata.resourcePath));
        entry.insert(QStringLiteral("assetPath"), WhatSon::Resources::normalizePath(metadata.assetPath));
        entry.insert(QStringLiteral("bucket"), metadata.bucket.trimmed());
        entry.insert(QStringLiteral("type"), metadata.type.trimmed().toCaseFolded());
        entry.insert(QStringLiteral("format"), WhatSon::Resources::normalizeFormat(metadata.format).toCaseFolded());
        return entry;
    }

    bool extractClipboardImage(const QMimeData* mimeData, QImage* outImage)
    {
        if (outImage != nullptr)
        {
            *outImage = QImage();
        }

        if (mimeData == nullptr)
        {
            return false;
        }

        auto tryLoadImageData = [&](const QByteArray& imageData)
        {
            if (imageData.isEmpty())
            {
                return false;
            }

            QImage image;
            if (!image.loadFromData(imageData) || image.isNull())
            {
                return false;
            }

            if (outImage != nullptr)
            {
                *outImage = image;
            }
            return true;
        };

        const QStringList mimeFormats = mimeData->formats();
        for (const QString& mimeFormat : mimeFormats)
        {
            if (!mimeFormat.startsWith(QStringLiteral("image/"), Qt::CaseInsensitive))
            {
                continue;
            }

            if (tryLoadImageData(mimeData->data(mimeFormat)))
            {
                return true;
            }
        }

        if (!mimeData->hasImage())
        {
            return false;
        }

        const QVariant imageVariant = mimeData->imageData();
        QImage image;
        if (imageVariant.canConvert<QImage>())
        {
            image = qvariant_cast<QImage>(imageVariant);
        }
        else if (imageVariant.canConvert<QPixmap>())
        {
            image = qvariant_cast<QPixmap>(imageVariant).toImage();
        }

        if (image.isNull())
        {
            return false;
        }

        if (outImage != nullptr)
        {
            *outImage = image;
        }
        return true;
    }

    bool clipboardContainsImportableImage()
    {
        const QClipboard* clipboard = QGuiApplication::clipboard();
        if (clipboard == nullptr)
        {
            return false;
        }

        return extractClipboardImage(clipboard->mimeData(), nullptr);
    }
}

ResourcesImportViewModel::ResourcesImportViewModel(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    if (QClipboard* clipboard = QGuiApplication::clipboard())
    {
        connect(
            clipboard,
            &QClipboard::dataChanged,
            this,
            &ResourcesImportViewModel::refreshClipboardImageAvailability);
    }
    refreshClipboardImageAvailability();
}

QString ResourcesImportViewModel::currentHubPath() const
{
    return m_currentHubPath;
}

void ResourcesImportViewModel::setCurrentHubPath(QString hubPath)
{
    hubPath = hubPath.trimmed().isEmpty()
                  ? QString()
                  : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (m_currentHubPath == hubPath)
    {
        return;
    }

    m_currentHubPath = std::move(hubPath);
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        QStringLiteral("setCurrentHubPath"),
        QStringLiteral("value=%1").arg(m_currentHubPath));
    emit currentHubPathChanged();
}

bool ResourcesImportViewModel::busy() const noexcept
{
    return m_busy;
}

bool ResourcesImportViewModel::clipboardImageAvailable() const noexcept
{
    return m_clipboardImageAvailable;
}

QString ResourcesImportViewModel::lastError() const
{
    return m_lastError;
}

void ResourcesImportViewModel::setReloadResourcesCallback(std::function<bool(const QString&, QString*)> callback)
{
    m_reloadResourcesCallback = std::move(callback);
}

bool ResourcesImportViewModel::canImportUrls(const QVariantList& urls) const
{
    if (m_busy || m_currentHubPath.trimmed().isEmpty())
    {
        return false;
    }

    return !extractDroppedLocalFiles(urls).isEmpty();
}

bool ResourcesImportViewModel::canImportDroppedUrls(const QVariantList& urls) const
{
    return canImportUrls(urls);
}

bool ResourcesImportViewModel::importUrls(const QVariantList& urls)
{
    return importUrlsInternal(urls, nullptr, true);
}

QVariantList ResourcesImportViewModel::importUrlsForEditor(const QVariantList& urls)
{
    QVariantList importedEntries;
    if (!importUrlsInternal(urls, &importedEntries, false))
    {
        return {};
    }
    return importedEntries;
}

bool ResourcesImportViewModel::importClipboardImage()
{
    return importClipboardImageInternal(nullptr, true);
}

QVariantList ResourcesImportViewModel::importClipboardImageForEditor()
{
    QVariantList importedEntries;
    if (!importClipboardImageInternal(&importedEntries, false))
    {
        return {};
    }
    return importedEntries;
}

bool ResourcesImportViewModel::importUrlsInternal(
    const QVariantList& urls,
    QVariantList* importedEntries,
    const bool reloadRuntime)
{
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        importedEntries == nullptr
            ? QStringLiteral("importUrls.begin")
            : QStringLiteral("importUrlsForEditor.begin"),
        QStringLiteral("urlCount=%1 hubPath=%2").arg(urls.size()).arg(m_currentHubPath));

    if (m_busy)
    {
        const QString errorMessage = QStringLiteral("Resource import is already running.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    if (m_currentHubPath.trimmed().isEmpty())
    {
        const QString errorMessage = QStringLiteral("Current hub path is empty.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    const QStringList sourceFiles = extractDroppedLocalFiles(urls);
    if (sourceFiles.isEmpty())
    {
        const QString errorMessage = QStringLiteral("Select at least one local file to import as a resource.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    QString resolveError;
    QString contentsDirectoryPath = resolveContentsDirectory(m_currentHubPath, &resolveError);
    if (contentsDirectoryPath.isEmpty())
    {
        setLastError(resolveError);
        emit operationFailed(resolveError);
        return false;
    }

    const QString resourcesFilePath = QDir(contentsDirectoryPath).filePath(QStringLiteral("Resources.wsresources"));
    const bool hadResourcesFile = QFileInfo(resourcesFilePath).isFile();
    QString previousResourcesFileText;
    if (hadResourcesFile && !readUtf8FileText(resourcesFilePath, &previousResourcesFileText, &resolveError))
    {
        setLastError(resolveError);
        emit operationFailed(resolveError);
        return false;
    }

    QStringList existingResourcePaths;
    if (!loadExistingResourcePaths(resourcesFilePath, &existingResourcePaths, &resolveError))
    {
        setLastError(resolveError);
        emit operationFailed(resolveError);
        return false;
    }

    const QString resourcesDirectoryPath = resolveResourcesDirectory(m_currentHubPath, existingResourcePaths, &resolveError);
    if (resourcesDirectoryPath.isEmpty())
    {
        setLastError(resolveError);
        emit operationFailed(resolveError);
        return false;
    }

    setBusy(true);
    setLastError(QString());

    QStringList importedResourcePaths;
    QStringList createdPackagePaths;
    importedResourcePaths.reserve(sourceFiles.size());
    createdPackagePaths.reserve(sourceFiles.size());
    QVariantList localImportedEntries;
    if (importedEntries != nullptr)
    {
        localImportedEntries.reserve(sourceFiles.size());
    }
    bool wroteResourcesFile = false;

    auto rollbackImportedResources = [&](const bool restoreResourcesFile, QString* rollbackError = nullptr)
    {
        QStringList rollbackErrors;

        for (const QString& createdPackagePath : std::as_const(createdPackagePaths))
        {
            if (!QFileInfo(createdPackagePath).exists())
            {
                continue;
            }

            if (!QDir(createdPackagePath).removeRecursively())
            {
                rollbackErrors.push_back(
                    QStringLiteral("Failed to remove imported resource package: %1").arg(createdPackagePath));
            }
        }

        if (restoreResourcesFile)
        {
            if (hadResourcesFile)
            {
                QString restoreError;
                if (!writeUtf8FileAtomically(resourcesFilePath, previousResourcesFileText, &restoreError))
                {
                    rollbackErrors.push_back(restoreError);
                }
            }
            else if (QFileInfo(resourcesFilePath).exists() && !QFile::remove(resourcesFilePath))
            {
                rollbackErrors.push_back(
                    QStringLiteral("Failed to remove restored Resources.wsresources file: %1").arg(resourcesFilePath));
            }
        }

        if (rollbackError != nullptr)
        {
            *rollbackError = rollbackErrors.join(QStringLiteral("; "));
        }
        return rollbackErrors.isEmpty();
    };

    for (const QString& sourceFilePath : sourceFiles)
    {
        QString resourcePath;
        QString packagePath;
        WhatSon::Resources::ResourcePackageMetadata importedMetadata;
        QString importError;
        if (!importSingleFile(
            sourceFilePath,
            resourcesDirectoryPath,
            &resourcePath,
            &packagePath,
            &importedMetadata,
            &importError))
        {
            rollbackImportedResources(false, nullptr);

            setBusy(false);
            setLastError(importError);
            emit operationFailed(importError);
            WhatSon::Debug::traceSelf(
                this,
                QString::fromLatin1(kScope),
                QStringLiteral("importUrls.failed"),
                QStringLiteral("reason=%1").arg(importError));
            return false;
        }

        importedResourcePaths.push_back(resourcePath);
        createdPackagePaths.push_back(packagePath);
        if (importedEntries != nullptr)
        {
            localImportedEntries.push_back(importedEntryFromMetadata(importedMetadata));
        }
    }

    QStringList mergedResourcePaths = existingResourcePaths;
    for (const QString& resourcePath : std::as_const(importedResourcePaths))
    {
        if (!mergedResourcePaths.contains(resourcePath))
        {
            mergedResourcePaths.push_back(resourcePath);
        }
    }

    WhatSonResourcesHierarchyStore store;
    store.setHubPath(m_currentHubPath);
    store.setResourcePaths(mergedResourcePaths);

    QString writeError;
    if (!store.writeToFile(resourcesFilePath, &writeError))
    {
        rollbackImportedResources(false, nullptr);

        setBusy(false);
        setLastError(writeError);
        emit operationFailed(writeError);
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kScope),
            QStringLiteral("importUrls.failed"),
            QStringLiteral("reason=%1").arg(writeError));
        return false;
    }
    wroteResourcesFile = true;

    if (reloadRuntime && m_reloadResourcesCallback)
    {
        QString reloadError;
        if (!m_reloadResourcesCallback(m_currentHubPath, &reloadError))
        {
            QString rollbackError;
            rollbackImportedResources(wroteResourcesFile, &rollbackError);
            const QString errorMessage = reloadError.trimmed().isEmpty()
                                             ? QStringLiteral("Imported resources but failed to refresh the workspace.")
                                             : QStringLiteral(
                                                   "Imported resources but failed to refresh the workspace: %1").arg(
                                                   reloadError.trimmed());
            const QString finalErrorMessage = rollbackError.trimmed().isEmpty()
                                                  ? errorMessage
                                                  : QStringLiteral("%1 Rollback: %2").arg(
                                                      errorMessage,
                                                      rollbackError.trimmed());
            setBusy(false);
            setLastError(finalErrorMessage);
            emit operationFailed(finalErrorMessage);
            WhatSon::Debug::traceSelf(
                this,
                QString::fromLatin1(kScope),
                QStringLiteral("importUrls.reloadFailed"),
                QStringLiteral("reason=%1").arg(finalErrorMessage));
            return false;
        }
    }

    setBusy(false);
    setLastError(QString());
    emit importCompleted(importedResourcePaths.size());
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        importedEntries == nullptr
            ? QStringLiteral("importUrls.success")
            : QStringLiteral("importUrlsForEditor.success"),
        QStringLiteral("importedCount=%1").arg(importedResourcePaths.size()));
    if (importedEntries != nullptr)
    {
        *importedEntries = localImportedEntries;
    }
    return true;
}

bool ResourcesImportViewModel::importClipboardImageInternal(
    QVariantList* importedEntries,
    const bool reloadRuntime)
{
    const QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard == nullptr)
    {
        const QString errorMessage = QStringLiteral("Clipboard service is unavailable.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    QImage clipboardImage;
    if (!extractClipboardImage(clipboard->mimeData(), &clipboardImage) || clipboardImage.isNull())
    {
        const QString errorMessage = QStringLiteral("Clipboard does not contain an importable image.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        refreshClipboardImageAvailability();
        return false;
    }

    QTemporaryDir temporaryDirectory;
    if (!temporaryDirectory.isValid())
    {
        const QString errorMessage = QStringLiteral("Failed to create a temporary directory for clipboard import.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    const QString temporaryImagePath = temporaryDirectory.filePath(QStringLiteral("clipboard-image.png"));
    if (!clipboardImage.save(temporaryImagePath, "PNG"))
    {
        const QString errorMessage = QStringLiteral("Failed to write the clipboard image into a temporary PNG file.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    const QVariantList temporaryUrls = {QUrl::fromLocalFile(temporaryImagePath)};
    const bool imported = importUrlsInternal(temporaryUrls, importedEntries, reloadRuntime);
    refreshClipboardImageAvailability();
    return imported;
}

bool ResourcesImportViewModel::importDroppedUrls(const QVariantList& urls)
{
    return importUrls(urls);
}

bool ResourcesImportViewModel::reloadImportedResources()
{
    if (!m_reloadResourcesCallback)
    {
        return true;
    }

    if (m_busy)
    {
        const QString errorMessage = QStringLiteral("Resource import is already running.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    if (m_currentHubPath.trimmed().isEmpty())
    {
        const QString errorMessage = QStringLiteral("Current hub path is empty.");
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    QString reloadError;
    if (!m_reloadResourcesCallback(m_currentHubPath, &reloadError))
    {
        const QString errorMessage = reloadError.trimmed().isEmpty()
            ? QStringLiteral("Failed to refresh imported resources.")
            : QStringLiteral("Failed to refresh imported resources: %1").arg(reloadError.trimmed());
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kScope),
            QStringLiteral("reloadImportedResources.failed"),
            QStringLiteral("reason=%1").arg(errorMessage));
        return false;
    }

    setLastError(QString());
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kScope),
        QStringLiteral("reloadImportedResources.success"),
        QStringLiteral("hubPath=%1").arg(m_currentHubPath));
    return true;
}

void ResourcesImportViewModel::setBusy(const bool busy)
{
    if (m_busy == busy)
    {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void ResourcesImportViewModel::refreshClipboardImageAvailability()
{
    const bool nextValue = clipboardContainsImportableImage();
    if (m_clipboardImageAvailable == nextValue)
    {
        return;
    }

    m_clipboardImageAvailable = nextValue;
    emit clipboardImageAvailableChanged();
}

void ResourcesImportViewModel::setLastError(QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    if (m_lastError == errorMessage)
    {
        return;
    }

    m_lastError = std::move(errorMessage);
    emit lastErrorChanged();
}
