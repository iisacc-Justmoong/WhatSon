#include "WhatSonStartupHubResolver.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"
#include "runtime/bootstrap/WhatSonAppLaunchSupport.hpp"
#include "store/hub/SelectedHubStore.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

namespace WhatSon::Runtime::Startup
{
    QString resolveStartupHubMountPath(
        const QString& hubPath,
        const QByteArray& hubAccessBookmark,
        QString* errorMessage)
    {
        const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                              ? QString()
                                              : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
        if (normalizedHubPath.isEmpty())
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
        QString restoredBookmarkPath;
        QString bookmarkRestoreError;
        if (!hubAccessBookmark.isEmpty())
        {
            WhatSon::Apple::SecurityScopedResourceAccess::restoreAccessFromBookmarkData(
                hubAccessBookmark,
                &restoredBookmarkPath,
                &bookmarkRestoreError);
        }

        QString iosAccessError;
        if (!WhatSon::Apple::SecurityScopedResourceAccess::ensureAccessForPath(normalizedHubPath, &iosAccessError))
        {
            const QString parentDirectoryPath = QFileInfo(normalizedHubPath).absolutePath();
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
#endif

        const QFileInfo hubInfo(normalizedHubPath);
        if (!hubInfo.exists() || !hubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Startup WhatSon Hub directory does not exist: %1").arg(normalizedHubPath);
            }
            return {};
        }

        if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
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

    StartupHubSelection resolveStartupHubSelection(
        SelectedHubStore& selectedHubStore,
        const QString& blueprintFallbackHubPath)
    {
        StartupHubSelection selection;
        const QString startupHubSelectionPath = selectedHubStore.startupHubPath(blueprintFallbackHubPath);
        const QByteArray startupHubAccessBookmark = selectedHubStore.selectedHubAccessBookmark();

        auto tryResolve = [&selection](const QString& selectionPath,
                                       const QByteArray& selectionBookmark,
                                       const QString& selectionLabel) -> bool
        {
            QString startupMountError;
            const QString resolvedStartupHubPath = resolveStartupHubMountPath(
                selectionPath,
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
            selection.accessBookmark = selectionBookmark;
            selection.mounted = true;
            return true;
        };

        if (!startupHubSelectionPath.isEmpty())
        {
            const bool startupSelectionResolved = tryResolve(
                startupHubSelectionPath,
                startupHubAccessBookmark,
                QStringLiteral("persisted"));
            if (!startupSelectionResolved
                && !blueprintFallbackHubPath.trimmed().isEmpty()
                && WhatSon::HubPath::normalizePath(startupHubSelectionPath)
                    != WhatSon::HubPath::normalizePath(blueprintFallbackHubPath))
            {
                tryResolve(blueprintFallbackHubPath, {}, QStringLiteral("blueprint fallback"));
            }
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
