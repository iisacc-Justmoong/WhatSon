#include "WhatSonStartupHubResolver.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"
#include "store/hub/ISelectedHubStore.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

namespace
{
    QString normalizedAbsolutePath(const QString& path)
    {
        return WhatSon::HubPath::normalizeAbsolutePath(path);
    }

    QString enclosingHubPackagePath(const QString& selectedPath)
    {
        const QString normalizedSelectedPath = normalizedAbsolutePath(selectedPath);
        if (normalizedSelectedPath.isEmpty())
        {
            return {};
        }

        const QFileInfo selectedInfo(normalizedSelectedPath);
        if (selectedInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            return normalizedSelectedPath;
        }

        QString candidatePath = selectedInfo.isDir()
                                    ? selectedInfo.absoluteFilePath()
                                    : selectedInfo.absolutePath();
        while (!candidatePath.isEmpty())
        {
            const QFileInfo candidateInfo(candidatePath);
            if (candidateInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
            {
                return normalizedAbsolutePath(candidateInfo.absoluteFilePath());
            }

            const QString parentPath = candidateInfo.absolutePath();
            if (parentPath.isEmpty() || parentPath == candidatePath)
            {
                break;
            }
            candidatePath = parentPath;
        }

        return {};
    }

    QString singleHubPackageInDirectory(const QString& directoryPath)
    {
        const QString normalizedDirectoryPath = normalizedAbsolutePath(directoryPath);
        if (normalizedDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir selectedDirectory(normalizedDirectoryPath);
        const QFileInfoList packageCandidates = selectedDirectory.entryInfoList(
            QStringList{QStringLiteral("*.wshub")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        if (packageCandidates.size() != 1)
        {
            return {};
        }

        return normalizedAbsolutePath(packageCandidates.constFirst().absoluteFilePath());
    }

    QString promotedHubPackagePath(const QString& selectedPath)
    {
        const QString enclosingHubPath = enclosingHubPackagePath(selectedPath);
        if (!enclosingHubPath.isEmpty())
        {
            return enclosingHubPath;
        }

        const QFileInfo selectedInfo(normalizedAbsolutePath(selectedPath));
        if (selectedInfo.isDir())
        {
            return singleHubPackageInDirectory(selectedInfo.absoluteFilePath());
        }

        return {};
    }
}

namespace WhatSon::Runtime::Startup
{
    QString resolveStartupHubMountPath(
        const QString& hubPath,
        const QString& hubSelectionUrl,
        const QByteArray& hubAccessBookmark,
        QString* errorMessage)
    {
        const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                              ? QString()
                                              : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
        const QString normalizedSelectionUrl = hubSelectionUrl.trimmed();
        if (normalizedHubPath.isEmpty() && normalizedSelectionUrl.isEmpty() && hubAccessBookmark.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return {};
        }

        if (WhatSon::Android::Storage::isSupportedUri(normalizedHubPath))
        {
            QString mountedHubPath;
            if (!WhatSon::Android::Storage::mountHub(normalizedHubPath, &mountedHubPath, errorMessage))
            {
                return {};
            }
            return WhatSon::HubPath::normalizeAbsolutePath(mountedHubPath);
        }

        if (WhatSon::Android::Storage::isMountedHubPath(normalizedHubPath))
        {
            const QString sourceUri = WhatSon::Android::Storage::mountedHubSourceUri(normalizedHubPath);
            if (sourceUri.trimmed().isEmpty())
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Stored Android mounted WhatSon Hub is missing its source document URI.");
                }
                return {};
            }

            QString remountedHubPath;
            if (!WhatSon::Android::Storage::mountHub(sourceUri, &remountedHubPath, errorMessage))
            {
                return {};
            }
            return WhatSon::HubPath::normalizeAbsolutePath(remountedHubPath);
        }

#if defined(Q_OS_IOS)
        QString iosResolvedHubPath = promotedHubPackagePath(normalizedHubPath);
        QString restoredBookmarkPath;
        QString bookmarkRestoreError;
        if (!hubAccessBookmark.isEmpty())
        {
            WhatSon::Apple::SecurityScopedResourceAccess::restoreAccessFromBookmarkData(
                hubAccessBookmark,
                &restoredBookmarkPath,
                &bookmarkRestoreError);
            const QString restoredBookmarkHubPath = promotedHubPackagePath(restoredBookmarkPath);
            if (!restoredBookmarkHubPath.isEmpty())
            {
                iosResolvedHubPath = restoredBookmarkHubPath;
            }
        }

        QString selectionUrlPathError;
        if (iosResolvedHubPath.isEmpty() && !normalizedSelectionUrl.isEmpty())
        {
            const QString selectionUrlPath = WhatSon::Apple::SecurityScopedResourceAccess::localPathForUrl(
                QUrl(normalizedSelectionUrl),
                false,
                &selectionUrlPathError);
            const QString resolvedSelectionUrlHubPath = promotedHubPackagePath(selectionUrlPath);
            if (!resolvedSelectionUrlHubPath.isEmpty())
            {
                iosResolvedHubPath = resolvedSelectionUrlHubPath;
            }
        }

        if (iosResolvedHubPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = !bookmarkRestoreError.trimmed().isEmpty()
                                    ? bookmarkRestoreError.trimmed()
                                    : (!selectionUrlPathError.trimmed().isEmpty()
                                           ? selectionUrlPathError.trimmed()
                                           : QStringLiteral("No mountable iOS WhatSon Hub path could be restored from the stored selection."));
            }
            return {};
        }

        QString iosAccessError;
        if (!WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(iosResolvedHubPath, &iosAccessError))
        {
            const QString parentDirectoryPath = QFileInfo(iosResolvedHubPath).absolutePath();
            QString parentAccessError;
            const bool parentAccessGranted =
                !parentDirectoryPath.trimmed().isEmpty()
                && WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(parentDirectoryPath, &parentAccessError);
            if (!parentAccessGranted)
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = iosAccessError.trimmed().isEmpty()
                                        ? (bookmarkRestoreError.trimmed().isEmpty()
                                               ? (parentAccessError.trimmed().isEmpty()
                                                      ? QStringLiteral("iOS denied access to the stored WhatSon Hub path during startup.")
                                                      : parentAccessError.trimmed())
                                               : bookmarkRestoreError.trimmed())
                                        : iosAccessError.trimmed();
                }
                return {};
            }
        }

        const QFileInfo iosResolvedHubInfo(iosResolvedHubPath);
        if (!iosResolvedHubInfo.exists() || !iosResolvedHubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Startup WhatSon Hub directory does not exist: %1").arg(iosResolvedHubPath);
            }
            return {};
        }

        if (!iosResolvedHubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Startup WhatSon Hub path is not a .wshub directory: %1").arg(iosResolvedHubPath);
            }
            return {};
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return iosResolvedHubPath;
#endif

        const QFileInfo normalizedHubInfo(normalizedHubPath);
        if (!normalizedHubInfo.exists() || !normalizedHubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Startup WhatSon Hub directory does not exist: %1").arg(normalizedHubPath);
            }
            return {};
        }

        if (!normalizedHubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Startup WhatSon Hub path is not a .wshub directory: %1").arg(normalizedHubPath);
            }
            return {};
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return normalizedHubPath;
    }

    StartupHubSelection resolveStartupHubSelection(ISelectedHubStore& selectedHubStore)
    {
        StartupHubSelection selection;
        const QString startupHubSelectionPath = selectedHubStore.startupHubPath();
        const QString startupHubSelectionUrl = selectedHubStore.startupHubUrl();
        const QByteArray startupHubAccessBookmark = selectedHubStore.selectedHubAccessBookmark();

        auto tryResolve = [&selection](const QString& selectionPath,
                                       const QString& selectionUrl,
                                       const QByteArray& selectionBookmark,
                                       const QString& selectionLabel) -> bool
        {
            QString startupMountError;
            const QString resolvedStartupHubPath = resolveStartupHubMountPath(
                selectionPath,
                selectionUrl,
                selectionBookmark,
                &startupMountError);
            if (resolvedStartupHubPath.isEmpty())
            {
                if (!startupMountError.trimmed().isEmpty())
                {
                    qWarning().noquote()
                        << QStringLiteral("Failed to resolve %1 startup WhatSon Hub mount '%2': %3")
                               .arg(selectionLabel, selectionPath, startupMountError.trimmed());
                }
                return false;
            }

            selection.hubPath = resolvedStartupHubPath;
            selection.selectionUrl = selectionUrl;
            selection.accessBookmark = selectionBookmark;
            selection.mounted = true;
            return true;
        };

        if (!startupHubSelectionPath.isEmpty() || !startupHubSelectionUrl.isEmpty() || !startupHubAccessBookmark.isEmpty())
        {
            tryResolve(
                startupHubSelectionPath,
                startupHubSelectionUrl,
                startupHubAccessBookmark,
                QStringLiteral("persisted"));
        }

        if (!selection.mounted)
        {
            qWarning().noquote() << QStringLiteral("No startup WhatSon Hub could be resolved.");
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadFromWshub.skipped"),
                QStringLiteral("no startup .wshub detected"));
        }

        return selection;
    }
}
