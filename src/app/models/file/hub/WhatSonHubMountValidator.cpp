#include "WhatSonHubMountValidator.hpp"

#include "WhatSonHubPathUtils.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    QString normalizeAbsolutePath(const QString& path)
    {
        return WhatSon::HubPath::normalizeAbsolutePath(path);
    }

    QString resolvePrimaryDirectoryEntry(
        const QDir& baseDirectory,
        const QString& fixedName,
        const QString& dynamicPattern)
    {
        const QString fixedPath = normalizeAbsolutePath(baseDirectory.filePath(fixedName));
        if (!fixedPath.isEmpty() && QFileInfo(fixedPath).isDir())
        {
            return fixedPath;
        }

        const QStringList dynamicEntries = baseDirectory.entryList(
            QStringList{dynamicPattern},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        if (dynamicEntries.isEmpty())
        {
            return {};
        }

        return normalizeAbsolutePath(baseDirectory.filePath(dynamicEntries.constFirst()));
    }

    QString resolvePrimaryFileEntry(const QDir& baseDirectory, const QStringList& nameFilters)
    {
        const QStringList entries = baseDirectory.entryList(
            nameFilters,
            QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name);
        if (entries.isEmpty())
        {
            return {};
        }

        return normalizeAbsolutePath(baseDirectory.filePath(entries.constFirst()));
    }

    bool requireEntryPath(
        const QString& basePath,
        const QString& entryName,
        const bool allowDirectory,
        const bool allowFile,
        QString* errorMessage)
    {
        const QString absoluteEntryPath = normalizeAbsolutePath(QDir(basePath).filePath(entryName));
        const QFileInfo entryInfo(absoluteEntryPath);
        if (!entryInfo.exists())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Required hub entry is missing: %1").arg(absoluteEntryPath);
            }
            return false;
        }
        if (entryInfo.isDir() && allowDirectory)
        {
            return true;
        }
        if (entryInfo.isFile() && allowFile)
        {
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub entry has an unexpected type: %1").arg(absoluteEntryPath);
        }
        return false;
    }

    WhatSonHubMountValidation failedMountValidation(const QString& failureMessage)
    {
        WhatSonHubMountValidation validation;
        validation.failureMessage = failureMessage.trimmed();
        return validation;
    }
} // namespace

WhatSonHubMountValidation WhatSonHubMountValidator::resolveMountedHub(
    const QString& hubPath,
    const QByteArray& hubAccessBookmark) const
{
    const QString normalizedHubPath = normalizeAbsolutePath(hubPath);
    if (normalizedHubPath.isEmpty())
    {
        return failedMountValidation(QStringLiteral("WhatSon Hub path must not be empty."));
    }

    QString mountedHubPath = normalizedHubPath;
    if (WhatSon::Android::Storage::isSupportedUri(normalizedHubPath))
    {
        QString mountError;
        if (!WhatSon::Android::Storage::mountHub(normalizedHubPath, &mountedHubPath, &mountError))
        {
            return failedMountValidation(
                mountError.trimmed().isEmpty()
                    ? QStringLiteral("Failed to mount the selected WhatSon Hub package.")
                    : mountError);
        }
        mountedHubPath = normalizeAbsolutePath(mountedHubPath);
    }
    else if (WhatSon::Android::Storage::isMountedHubPath(normalizedHubPath))
    {
        const QString sourceUri = WhatSon::Android::Storage::mountedHubSourceUri(normalizedHubPath);
        if (sourceUri.trimmed().isEmpty())
        {
            return failedMountValidation(
                QStringLiteral("Stored Android mounted WhatSon Hub is missing its source document URI."));
        }

        QString remountError;
        if (!WhatSon::Android::Storage::mountHub(sourceUri, &mountedHubPath, &remountError))
        {
            return failedMountValidation(
                remountError.trimmed().isEmpty()
                    ? QStringLiteral("Failed to remount the stored WhatSon Hub package.")
                    : remountError);
        }
        mountedHubPath = normalizeAbsolutePath(mountedHubPath);
    }

    if (WhatSon::HubPath::isNonLocalUrl(mountedHubPath))
    {
        return failedMountValidation(
            QStringLiteral("The selected WhatSon Hub provider does not expose a mountable local directory path."));
    }

#if defined(Q_OS_IOS)
    QString bookmarkRestoreError;
    if (!hubAccessBookmark.isEmpty())
    {
        QString restoredBookmarkPath;
        WhatSon::Apple::SecurityScopedResourceAccess::restoreAccessFromBookmarkData(
            hubAccessBookmark,
            &restoredBookmarkPath,
            &bookmarkRestoreError);
    }

    QString iosAccessError;
    if (!WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(mountedHubPath, &iosAccessError))
    {
        const QString parentDirectoryPath = QFileInfo(mountedHubPath).absolutePath();
        QString parentAccessError;
        const bool parentAccessGranted =
            !parentDirectoryPath.trimmed().isEmpty()
            && WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(
                parentDirectoryPath,
                &parentAccessError);
        if (!parentAccessGranted)
        {
            return failedMountValidation(
                iosAccessError.trimmed().isEmpty()
                    ? (bookmarkRestoreError.trimmed().isEmpty()
                           ? (parentAccessError.trimmed().isEmpty()
                                  ? QStringLiteral(
                                        "iOS denied access to the selected WhatSon Hub path.")
                                  : parentAccessError.trimmed())
                           : bookmarkRestoreError.trimmed())
                    : iosAccessError.trimmed());
        }
    }
#endif

    mountedHubPath = normalizeAbsolutePath(mountedHubPath);
    const QFileInfo hubInfo(mountedHubPath);
    if (!hubInfo.exists() || !hubInfo.isDir())
    {
        return failedMountValidation(
            QStringLiteral("Resolved WhatSon Hub directory does not exist: %1").arg(mountedHubPath));
    }

    if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
    {
        return failedMountValidation(
            QStringLiteral("Resolved path is not a .wshub directory: %1").arg(mountedHubPath));
    }

    const QDir hubDirectory(mountedHubPath);
    const QString contentsPath = resolvePrimaryDirectoryEntry(
        hubDirectory,
        QStringLiteral(".wscontents"),
        QStringLiteral("*.wscontents"));
    if (contentsPath.isEmpty())
    {
        return failedMountValidation(
            QStringLiteral("No *.wscontents directory found inside hub: %1").arg(mountedHubPath));
    }

    const QDir contentsDirectory(contentsPath);
    const QString libraryPath = resolvePrimaryDirectoryEntry(
        contentsDirectory,
        QStringLiteral("Library.wslibrary"),
        QStringLiteral("*.wslibrary"));
    if (libraryPath.isEmpty())
    {
        return failedMountValidation(
            QStringLiteral("Library.wslibrary directory is missing: %1").arg(contentsPath));
    }

    const QString resourcesPath = resolvePrimaryDirectoryEntry(
        hubDirectory,
        QStringLiteral(".wsresources"),
        QStringLiteral("*.wsresources"));
    if (resourcesPath.isEmpty())
    {
        return failedMountValidation(
            QStringLiteral("No *.wsresources directory found inside hub: %1").arg(mountedHubPath));
    }

    if (resolvePrimaryFileEntry(hubDirectory, QStringList{QStringLiteral("*.wsstat")}).isEmpty())
    {
        return failedMountValidation(
            QStringLiteral("No *.wsstat file found inside hub: %1").arg(mountedHubPath));
    }

    QString entryError;
    if (!requireEntryPath(contentsPath, QStringLiteral("Folders.wsfolders"), false, true, &entryError)
        || !requireEntryPath(contentsPath, QStringLiteral("ProjectLists.wsproj"), false, true, &entryError)
        || !requireEntryPath(contentsPath, QStringLiteral("Bookmarks.wsbookmarks"), false, true, &entryError)
        || !requireEntryPath(contentsPath, QStringLiteral("Tags.wstags"), false, true, &entryError)
        || !requireEntryPath(contentsPath, QStringLiteral("Progress.wsprogress"), false, true, &entryError)
        || !requireEntryPath(contentsPath, QStringLiteral("Preset.wspreset"), true, true, &entryError)
        || !requireEntryPath(libraryPath, QStringLiteral("index.wsnindex"), false, true, &entryError))
    {
        return failedMountValidation(entryError);
    }

    WhatSonHubMountValidation validation;
    validation.mounted = true;
    validation.hubPath = mountedHubPath;
    return validation;
}
