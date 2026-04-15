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
#include <QHash>
#include <QMetaType>
#include <QMimeData>
#include <QPixmap>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSequentialIterable>
#include <QSet>
#include <QUuid>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>

#include <utility>

namespace
{
    constexpr auto kScope = "resources.import";

    enum class ImportConflictPolicyValue
    {
        Abort = 0,
        Overwrite = 1,
        KeepBoth = 2
    };

    struct ExistingResourcePackageEntry final
    {
        QString assetFileName;
        WhatSon::Resources::ResourcePackageMetadata metadata;
        QString packageDirectoryPath;
        QString resourcePath;
    };

    struct ImportConflictDescriptor final
    {
        QString existingAssetFileName;
        WhatSon::Resources::ResourcePackageMetadata existingMetadata;
        QString packageDirectoryPath;
        QString resourcePath;
        QString sourceFileName;
        QString sourceFilePath;

        bool valid() const
        {
            return !existingAssetFileName.trimmed().isEmpty()
                && !packageDirectoryPath.trimmed().isEmpty()
                && !sourceFileName.trimmed().isEmpty();
        }
    };

    struct OverwrittenPackageBackup final
    {
        QString backupDirectoryPath;
        QString packageDirectoryPath;
    };

    ImportConflictPolicyValue normalizedImportConflictPolicy(const int conflictPolicy)
    {
        switch (conflictPolicy)
        {
        case ResourcesImportViewModel::ConflictPolicyOverwrite:
            return ImportConflictPolicyValue::Overwrite;
        case ResourcesImportViewModel::ConflictPolicyKeepBoth:
            return ImportConflictPolicyValue::KeepBoth;
        default:
            return ImportConflictPolicyValue::Abort;
        }
    }

    QVariantMap emptyImportConflictMap()
    {
        return QVariantMap {
            {QStringLiteral("conflict"), false}
        };
    }

    QVariantMap importConflictMap(const ImportConflictDescriptor& descriptor)
    {
        if (!descriptor.valid())
        {
            return emptyImportConflictMap();
        }

        QVariantMap result;
        result.insert(QStringLiteral("conflict"), true);
        result.insert(QStringLiteral("sourceFileName"), descriptor.sourceFileName);
        result.insert(QStringLiteral("sourceFilePath"), descriptor.sourceFilePath);
        result.insert(QStringLiteral("existingAssetFileName"), descriptor.existingAssetFileName);
        result.insert(QStringLiteral("existingPackageDirectoryPath"), descriptor.packageDirectoryPath);
        result.insert(QStringLiteral("existingResourcePath"), descriptor.resourcePath);
        result.insert(QStringLiteral("existingResourceId"), descriptor.existingMetadata.resourceId.trimmed());
        result.insert(QStringLiteral("existingAssetPath"), descriptor.existingMetadata.assetPath.trimmed());
        result.insert(QStringLiteral("existingType"), descriptor.existingMetadata.type.trimmed());
        result.insert(QStringLiteral("existingFormat"), descriptor.existingMetadata.format.trimmed());
        return result;
    }

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

    bool loadExistingResourcePackageEntries(
        const QString& resourcesDirectoryPath,
        QList<ExistingResourcePackageEntry>* outEntries,
        QString* errorMessage = nullptr)
    {
        if (outEntries == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outEntries must not be null.");
            }
            return false;
        }

        outEntries->clear();
        const QFileInfoList packageDirectories = QDir(resourcesDirectoryPath).entryInfoList(
            QStringList{QStringLiteral("*.wsresource")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);

        for (const QFileInfo& packageDirectoryInfo : packageDirectories)
        {
            ExistingResourcePackageEntry entry;
            entry.packageDirectoryPath = WhatSon::Resources::normalizePath(packageDirectoryInfo.absoluteFilePath());
            entry.resourcePath = WhatSon::Resources::resourcePathForPackageDirectory(entry.packageDirectoryPath);
            QString metadataError;
            if (!WhatSon::Resources::loadResourcePackageMetadata(
                entry.packageDirectoryPath,
                &entry.metadata,
                &metadataError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = metadataError;
                }
                return false;
            }

            if (entry.metadata.resourcePath.trimmed().isEmpty())
            {
                entry.metadata.resourcePath = entry.resourcePath;
            }
            entry.assetFileName = QFileInfo(entry.metadata.assetPath.trimmed()).fileName().trimmed();
            if (entry.assetFileName.isEmpty())
            {
                entry.assetFileName = QFileInfo(entry.packageDirectoryPath).fileName().trimmed();
            }
            outEntries->push_back(entry);
        }

        return true;
    }

    bool findFirstImportConflict(
        const QStringList& sourceFileNames,
        const QStringList& sourceFilePaths,
        const QString& resourcesDirectoryPath,
        ImportConflictDescriptor* outDescriptor,
        QString* errorMessage = nullptr)
    {
        if (outDescriptor != nullptr)
        {
            *outDescriptor = {};
        }

        QList<ExistingResourcePackageEntry> existingEntries;
        if (!loadExistingResourcePackageEntries(resourcesDirectoryPath, &existingEntries, errorMessage))
        {
            return false;
        }

        QHash<QString, ExistingResourcePackageEntry> entriesByFileName;
        for (const ExistingResourcePackageEntry& entry : std::as_const(existingEntries))
        {
            const QString lookupKey = entry.assetFileName.trimmed().toCaseFolded();
            if (lookupKey.isEmpty() || entriesByFileName.contains(lookupKey))
            {
                continue;
            }
            entriesByFileName.insert(lookupKey, entry);
        }

        const int entryCount = std::min(sourceFileNames.size(), sourceFilePaths.size());
        for (int index = 0; index < entryCount; ++index)
        {
            const QString sourceFileName = sourceFileNames.at(index).trimmed();
            const QString lookupKey = sourceFileName.toCaseFolded();
            if (lookupKey.isEmpty() || !entriesByFileName.contains(lookupKey))
            {
                continue;
            }

            const ExistingResourcePackageEntry existingEntry = entriesByFileName.value(lookupKey);
            if (outDescriptor != nullptr)
            {
                outDescriptor->existingAssetFileName = existingEntry.assetFileName;
                outDescriptor->existingMetadata = existingEntry.metadata;
                outDescriptor->packageDirectoryPath = existingEntry.packageDirectoryPath;
                outDescriptor->resourcePath = existingEntry.resourcePath;
                outDescriptor->sourceFileName = sourceFileName;
                outDescriptor->sourceFilePath = sourceFilePaths.at(index).trimmed();
            }
            return true;
        }

        return true;
    }

    bool findFirstImportConflict(
        const QStringList& sourceFiles,
        const QString& resourcesDirectoryPath,
        ImportConflictDescriptor* outDescriptor,
        QString* errorMessage = nullptr)
    {
        QStringList sourceFileNames;
        QStringList normalizedSourcePaths;
        sourceFileNames.reserve(sourceFiles.size());
        normalizedSourcePaths.reserve(sourceFiles.size());
        for (const QString& sourceFilePath : sourceFiles)
        {
            const QString sourceFileName = QFileInfo(sourceFilePath).fileName().trimmed();
            sourceFileNames.push_back(sourceFileName);
            normalizedSourcePaths.push_back(WhatSon::HubPath::normalizeAbsolutePath(sourceFilePath));
        }
        return findFirstImportConflict(
            sourceFileNames,
            normalizedSourcePaths,
            resourcesDirectoryPath,
            outDescriptor,
            errorMessage);
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

    QString duplicateImportResolutionRequiredMessage(const ImportConflictDescriptor& descriptor)
    {
        const QString fileName = descriptor.sourceFileName.trimmed().isEmpty()
            ? descriptor.existingAssetFileName.trimmed()
            : descriptor.sourceFileName.trimmed();
        return QStringLiteral(
                   "A resource named \"%1\" already exists. Choose overwrite, keep both, or cancel from the duplicate import alert.")
            .arg(fileName);
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

    bool overwriteSingleFile(
        const QString& sourceFilePath,
        const ImportConflictDescriptor& conflictDescriptor,
        const QString& resourcesDirectoryPath,
        QString* outResourcePath,
        QString* outCreatedPackagePath,
        WhatSon::Resources::ResourcePackageMetadata* outMetadata,
        QString* outBackupDirectoryPath,
        QString* errorMessage = nullptr)
    {
        if (outResourcePath == nullptr
            || outCreatedPackagePath == nullptr
            || outMetadata == nullptr
            || outBackupDirectoryPath == nullptr)
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
        *outBackupDirectoryPath = QString();

        const QFileInfo sourceFileInfo(sourceFilePath);
        if (!sourceFileInfo.exists() || !sourceFileInfo.isFile())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Dropped file does not exist: %1").arg(sourceFilePath);
            }
            return false;
        }

        const QString packageDirectoryPath =
            WhatSon::Resources::normalizePath(conflictDescriptor.packageDirectoryPath);
        if (!QFileInfo(packageDirectoryPath).isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Existing resource package is missing: %1").arg(packageDirectoryPath);
            }
            return false;
        }

        const QString backupDirectoryPath = QDir(resourcesDirectoryPath).filePath(
            QStringLiteral(".import-backup-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
        if (!QDir().rename(packageDirectoryPath, backupDirectoryPath))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to stage the existing resource package for overwrite: %1").arg(
                    packageDirectoryPath);
            }
            return false;
        }

        const auto restoreBackupAndFail = [&](const QString& failureText)
        {
            if (QFileInfo(packageDirectoryPath).exists())
            {
                QDir(packageDirectoryPath).removeRecursively();
            }
            QDir().rename(backupDirectoryPath, packageDirectoryPath);
            if (errorMessage != nullptr)
            {
                *errorMessage = failureText;
            }
            return false;
        };

        if (!QDir().mkpath(packageDirectoryPath))
        {
            return restoreBackupAndFail(
                QStringLiteral("Failed to recreate overwritten resource package directory: %1").arg(
                    packageDirectoryPath));
        }

        const QString destinationAssetPath = QDir(packageDirectoryPath).filePath(sourceFileInfo.fileName());
        if (!QFile::copy(sourceFilePath, destinationAssetPath))
        {
            return restoreBackupAndFail(
                QStringLiteral("Failed to copy dropped file into overwritten resource package: %1").arg(
                    sourceFilePath));
        }

        WhatSon::Resources::ResourcePackageMetadata metadata =
            WhatSon::Resources::buildMetadataForAssetFile(
                sourceFileInfo.fileName(),
                conflictDescriptor.existingMetadata.resourceId.trimmed(),
                conflictDescriptor.resourcePath.trimmed());
        if (metadata.resourcePath.trimmed().isEmpty())
        {
            metadata.resourcePath = conflictDescriptor.resourcePath.trimmed();
        }

        QString writeError;
        if (!writeUtf8FileAtomically(
            QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName()),
            WhatSon::Resources::createResourcePackageMetadataXml(metadata),
            &writeError))
        {
            return restoreBackupAndFail(writeError);
        }

        *outResourcePath = metadata.resourcePath.trimmed().isEmpty()
            ? conflictDescriptor.resourcePath.trimmed()
            : metadata.resourcePath.trimmed();
        *outCreatedPackagePath = packageDirectoryPath;
        *outMetadata = metadata;
        *outBackupDirectoryPath = WhatSon::Resources::normalizePath(backupDirectoryPath);
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

    bool mimeFormatLooksLikeImage(const QString& mimeFormat)
    {
        const QString normalizedFormat = mimeFormat.trimmed().toCaseFolded();
        if (normalizedFormat.startsWith(QStringLiteral("image/")))
        {
            return true;
        }

        return normalizedFormat.contains(QStringLiteral("png"))
            || normalizedFormat.contains(QStringLiteral("jpeg"))
            || normalizedFormat.contains(QStringLiteral("jpg"))
            || normalizedFormat.contains(QStringLiteral("gif"))
            || normalizedFormat.contains(QStringLiteral("bmp"))
            || normalizedFormat.contains(QStringLiteral("webp"))
            || normalizedFormat.contains(QStringLiteral("tiff"))
            || normalizedFormat.contains(QStringLiteral("heic"))
            || normalizedFormat.contains(QStringLiteral("heif"));
    }

    QString firstClipboardImageDataUrl(const QMimeData* mimeData)
    {
        if (mimeData == nullptr)
        {
            return {};
        }

        const auto extractFromText = [](const QString& text) -> QString
        {
            const QString trimmedText = text.trimmed();
            if (trimmedText.startsWith(QStringLiteral("data:image/"), Qt::CaseInsensitive))
            {
                return trimmedText;
            }

            static const QRegularExpression quotedImageSrcPattern(
                QStringLiteral(R"(src\s*=\s*["'](data:image\/[^"']+)["'])"),
                QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = quotedImageSrcPattern.match(text);
            if (match.hasMatch())
            {
                return match.captured(1).trimmed();
            }

            static const QRegularExpression bareDataUrlPattern(
                QStringLiteral(R"((data:image\/[^\s"'<>]+))"),
                QRegularExpression::CaseInsensitiveOption);
            match = bareDataUrlPattern.match(text);
            if (match.hasMatch())
            {
                return match.captured(1).trimmed();
            }
            return {};
        };

        if (mimeData->hasHtml())
        {
            const QString htmlDataUrl = extractFromText(mimeData->html());
            if (!htmlDataUrl.isEmpty())
            {
                return htmlDataUrl;
            }
        }

        if (mimeData->hasText())
        {
            const QString textDataUrl = extractFromText(mimeData->text());
            if (!textDataUrl.isEmpty())
            {
                return textDataUrl;
            }
        }

        const QStringList mimeFormats = mimeData->formats();
        for (const QString& mimeFormat : mimeFormats)
        {
            if (!mimeFormat.startsWith(QStringLiteral("text/"), Qt::CaseInsensitive))
            {
                continue;
            }

            const QString payloadText = QString::fromUtf8(mimeData->data(mimeFormat));
            const QString dataUrl = extractFromText(payloadText);
            if (!dataUrl.isEmpty())
            {
                return dataUrl;
            }
        }

        return {};
    }

    QByteArray decodedClipboardImageDataUrlPayload(const QString& dataUrl)
    {
        const QString normalizedDataUrl = dataUrl.trimmed();
        if (!normalizedDataUrl.startsWith(QStringLiteral("data:image/"), Qt::CaseInsensitive))
        {
            return {};
        }

        const int commaIndex = normalizedDataUrl.indexOf(QLatin1Char(','));
        if (commaIndex <= 0)
        {
            return {};
        }

        const QString header = normalizedDataUrl.left(commaIndex);
        QString payload = normalizedDataUrl.mid(commaIndex + 1);
        if (payload.isEmpty())
        {
            return {};
        }

        if (header.contains(QStringLiteral(";base64"), Qt::CaseInsensitive))
        {
            payload.remove(QRegularExpression(QStringLiteral("\\s+")));
            return QByteArray::fromBase64(payload.toUtf8());
        }

        return QByteArray::fromPercentEncoding(payload.toUtf8());
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
            if (!mimeFormatLooksLikeImage(mimeFormat))
            {
                continue;
            }

            if (tryLoadImageData(mimeData->data(mimeFormat)))
            {
                return true;
            }
        }

        const QString clipboardImageDataUrl = firstClipboardImageDataUrl(mimeData);
        if (!clipboardImageDataUrl.isEmpty())
        {
            if (tryLoadImageData(decodedClipboardImageDataUrlPayload(clipboardImageDataUrl)))
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

    bool extractClipboardImage(const QClipboard* clipboard, QImage* outImage)
    {
        if (outImage != nullptr)
        {
            *outImage = QImage();
        }

        if (clipboard == nullptr)
        {
            return false;
        }

        if (extractClipboardImage(clipboard->mimeData(), outImage))
        {
            return true;
        }

        const QImage clipboardImage = clipboard->image();
        if (!clipboardImage.isNull())
        {
            if (outImage != nullptr)
            {
                *outImage = clipboardImage;
            }
            return true;
        }

        const QPixmap clipboardPixmap = clipboard->pixmap();
        if (!clipboardPixmap.isNull())
        {
            if (outImage != nullptr)
            {
                *outImage = clipboardPixmap.toImage();
            }
            return true;
        }

        return false;
    }

    bool clipboardContainsImportableImage()
    {
        const QClipboard* clipboard = QGuiApplication::clipboard();
        if (clipboard == nullptr)
        {
            return false;
        }

        return extractClipboardImage(clipboard, nullptr);
    }
}

ResourcesImportViewModel::ResourcesImportViewModel(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    connect(
        qApp,
        &QGuiApplication::applicationStateChanged,
        this,
        [this](Qt::ApplicationState)
        {
            refreshClipboardImageAvailability();
        });
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

QVariantMap ResourcesImportViewModel::inspectImportConflictForUrls(const QVariantList& urls) const
{
    if (m_busy || m_currentHubPath.trimmed().isEmpty())
    {
        return emptyImportConflictMap();
    }

    const QStringList sourceFiles = extractDroppedLocalFiles(urls);
    if (sourceFiles.isEmpty())
    {
        return emptyImportConflictMap();
    }

    QString resolveError;
    QStringList existingResourcePaths;
    const QString contentsDirectoryPath = resolveContentsDirectory(m_currentHubPath, &resolveError);
    if (contentsDirectoryPath.isEmpty())
    {
        return emptyImportConflictMap();
    }

    const QString resourcesFilePath = QDir(contentsDirectoryPath).filePath(QStringLiteral("Resources.wsresources"));
    if (!loadExistingResourcePaths(resourcesFilePath, &existingResourcePaths, &resolveError))
    {
        return emptyImportConflictMap();
    }

    const QString resourcesDirectoryPath =
        resolveResourcesDirectory(m_currentHubPath, existingResourcePaths, &resolveError);
    if (resourcesDirectoryPath.isEmpty())
    {
        return emptyImportConflictMap();
    }

    ImportConflictDescriptor descriptor;
    if (!findFirstImportConflict(sourceFiles, resourcesDirectoryPath, &descriptor, &resolveError))
    {
        return emptyImportConflictMap();
    }
    return importConflictMap(descriptor);
}

QVariantMap ResourcesImportViewModel::inspectClipboardImageImportConflict() const
{
    if (m_busy || m_currentHubPath.trimmed().isEmpty())
    {
        return emptyImportConflictMap();
    }

    if (!clipboardContainsImportableImage())
    {
        return emptyImportConflictMap();
    }

    QString resolveError;
    QStringList existingResourcePaths;
    const QString contentsDirectoryPath = resolveContentsDirectory(m_currentHubPath, &resolveError);
    if (contentsDirectoryPath.isEmpty())
    {
        return emptyImportConflictMap();
    }

    const QString resourcesFilePath = QDir(contentsDirectoryPath).filePath(QStringLiteral("Resources.wsresources"));
    if (!loadExistingResourcePaths(resourcesFilePath, &existingResourcePaths, &resolveError))
    {
        return emptyImportConflictMap();
    }

    const QString resourcesDirectoryPath =
        resolveResourcesDirectory(m_currentHubPath, existingResourcePaths, &resolveError);
    if (resourcesDirectoryPath.isEmpty())
    {
        return emptyImportConflictMap();
    }

    ImportConflictDescriptor descriptor;
    if (!findFirstImportConflict(
        QStringList{QStringLiteral("clipboard-image.png")},
        QStringList{QStringLiteral("clipboard-image.png")},
        resourcesDirectoryPath,
        &descriptor,
        &resolveError))
    {
        return emptyImportConflictMap();
    }

    return importConflictMap(descriptor);
}

bool ResourcesImportViewModel::importUrls(const QVariantList& urls)
{
    return importUrlsInternal(urls, nullptr, true, ConflictPolicyAbort);
}

bool ResourcesImportViewModel::importUrlsWithConflictPolicy(const QVariantList& urls, const int conflictPolicy)
{
    return importUrlsInternal(urls, nullptr, true, conflictPolicy);
}

QVariantList ResourcesImportViewModel::importUrlsForEditor(const QVariantList& urls)
{
    QVariantList importedEntries;
    if (!importUrlsInternal(urls, &importedEntries, false, ConflictPolicyAbort))
    {
        return {};
    }
    return importedEntries;
}

QVariantList ResourcesImportViewModel::importUrlsForEditorWithConflictPolicy(
    const QVariantList& urls,
    const int conflictPolicy)
{
    QVariantList importedEntries;
    if (!importUrlsInternal(urls, &importedEntries, false, conflictPolicy))
    {
        return {};
    }
    return importedEntries;
}

bool ResourcesImportViewModel::importClipboardImage()
{
    return importClipboardImageInternal(nullptr, true, ConflictPolicyAbort);
}

bool ResourcesImportViewModel::importClipboardImageWithConflictPolicy(const int conflictPolicy)
{
    return importClipboardImageInternal(nullptr, true, conflictPolicy);
}

QVariantList ResourcesImportViewModel::importClipboardImageForEditor()
{
    QVariantList importedEntries;
    if (!importClipboardImageInternal(&importedEntries, false, ConflictPolicyAbort))
    {
        return {};
    }
    return importedEntries;
}

QVariantList ResourcesImportViewModel::importClipboardImageForEditorWithConflictPolicy(const int conflictPolicy)
{
    QVariantList importedEntries;
    if (!importClipboardImageInternal(&importedEntries, false, conflictPolicy))
    {
        return {};
    }
    return importedEntries;
}

bool ResourcesImportViewModel::refreshClipboardImageAvailabilitySnapshot()
{
    refreshClipboardImageAvailability();
    return m_clipboardImageAvailable;
}

bool ResourcesImportViewModel::importUrlsInternal(
    const QVariantList& urls,
    QVariantList* importedEntries,
    const bool reloadRuntime,
    const int conflictPolicy)
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

    ImportConflictDescriptor conflictDescriptor;
    if (!findFirstImportConflict(sourceFiles, resourcesDirectoryPath, &conflictDescriptor, &resolveError))
    {
        setLastError(resolveError);
        emit operationFailed(resolveError);
        return false;
    }

    const ImportConflictPolicyValue normalizedConflictPolicy = normalizedImportConflictPolicy(conflictPolicy);
    if (conflictDescriptor.valid() && normalizedConflictPolicy == ImportConflictPolicyValue::Abort)
    {
        const QString errorMessage = duplicateImportResolutionRequiredMessage(conflictDescriptor);
        setLastError(errorMessage);
        emit operationFailed(errorMessage);
        return false;
    }

    setBusy(true);
    setLastError(QString());

    QStringList importedResourcePaths;
    QStringList createdPackagePaths;
    QList<OverwrittenPackageBackup> overwrittenPackageBackups;
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

        for (const OverwrittenPackageBackup& backup : std::as_const(overwrittenPackageBackups))
        {
            if (!backup.packageDirectoryPath.trimmed().isEmpty()
                && QFileInfo(backup.packageDirectoryPath).exists()
                && !QDir(backup.packageDirectoryPath).removeRecursively())
            {
                rollbackErrors.push_back(
                    QStringLiteral("Failed to clear overwritten resource package: %1").arg(
                        backup.packageDirectoryPath));
            }

            if (!backup.backupDirectoryPath.trimmed().isEmpty()
                && QFileInfo(backup.backupDirectoryPath).exists()
                && !QDir().rename(backup.backupDirectoryPath, backup.packageDirectoryPath))
            {
                rollbackErrors.push_back(
                    QStringLiteral("Failed to restore overwritten resource package: %1").arg(
                        backup.packageDirectoryPath));
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
        ImportConflictDescriptor sourceFileConflictDescriptor;
        if (!findFirstImportConflict(QStringList{sourceFilePath}, resourcesDirectoryPath, &sourceFileConflictDescriptor, &resolveError))
        {
            rollbackImportedResources(false, nullptr);

            setBusy(false);
            setLastError(resolveError);
            emit operationFailed(resolveError);
            return false;
        }

        QString resourcePath;
        QString packagePath;
        WhatSon::Resources::ResourcePackageMetadata importedMetadata;
        QString backupDirectoryPath;
        QString importError;
        const bool shouldOverwrite =
            sourceFileConflictDescriptor.valid()
            && normalizedConflictPolicy == ImportConflictPolicyValue::Overwrite;
        const bool imported =
            shouldOverwrite
                ? overwriteSingleFile(
                    sourceFilePath,
                    sourceFileConflictDescriptor,
                    resourcesDirectoryPath,
                    &resourcePath,
                    &packagePath,
                    &importedMetadata,
                    &backupDirectoryPath,
                    &importError)
                : importSingleFile(
                    sourceFilePath,
                    resourcesDirectoryPath,
                    &resourcePath,
                    &packagePath,
                    &importedMetadata,
                    &importError);
        if (!imported)
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
        if (shouldOverwrite)
        {
            bool existingBackupTracked = false;
            for (const OverwrittenPackageBackup& backup : std::as_const(overwrittenPackageBackups))
            {
                if (backup.packageDirectoryPath == packagePath)
                {
                    existingBackupTracked = true;
                    break;
                }
            }

            if (existingBackupTracked)
            {
                if (!backupDirectoryPath.trimmed().isEmpty() && QFileInfo(backupDirectoryPath).exists())
                {
                    QDir(backupDirectoryPath).removeRecursively();
                }
            }
            else
            {
                overwrittenPackageBackups.push_back(OverwrittenPackageBackup{
                    backupDirectoryPath,
                    packagePath
                });
            }
        }
        else
        {
            createdPackagePaths.push_back(packagePath);
        }
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
    for (const OverwrittenPackageBackup& backup : std::as_const(overwrittenPackageBackups))
    {
        if (!backup.backupDirectoryPath.trimmed().isEmpty() && QFileInfo(backup.backupDirectoryPath).exists())
        {
            QDir(backup.backupDirectoryPath).removeRecursively();
        }
    }
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
    const bool reloadRuntime,
    const int conflictPolicy)
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
    if (!extractClipboardImage(clipboard, &clipboardImage) || clipboardImage.isNull())
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
    const bool imported = importUrlsInternal(temporaryUrls, importedEntries, reloadRuntime, conflictPolicy);
    refreshClipboardImageAvailability();
    return imported;
}

bool ResourcesImportViewModel::importDroppedUrls(const QVariantList& urls)
{
    return importUrlsWithConflictPolicy(urls, ConflictPolicyAbort);
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
