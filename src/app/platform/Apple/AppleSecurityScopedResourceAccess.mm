#include "AppleSecurityScopedResourceAccess.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QHash>

#if defined(Q_OS_IOS)
#import <Foundation/Foundation.h>
#endif

namespace WhatSon::Apple::SecurityScopedResourceAccess
{
#if defined(Q_OS_IOS)
namespace
{
    QString normalizeAbsolutePath(const QString& path)
    {
        if (path.trimmed().isEmpty())
        {
            return QString();
        }

        return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    }

    bool pathExists(const QString& localPath)
    {
        return QFileInfo(localPath).exists() || QDir(localPath).exists();
    }

    class ScopedResourceRegistry final
    {
    public:
        ~ScopedResourceRegistry()
        {
            for (auto it = m_urls.begin(); it != m_urls.end(); ++it)
            {
                NSURL* url = it.value();
                if (url == nil)
                {
                    continue;
                }

                [url stopAccessingSecurityScopedResource];
                [url release];
            }
        }

        [[nodiscard]] bool contains(const QString& localPath) const
        {
            return m_urls.contains(localPath);
        }

        [[nodiscard]] NSURL* urlForPath(const QString& localPath) const
        {
            return m_urls.value(localPath, nil);
        }

        void retain(const QString& localPath, NSURL* url)
        {
            if (localPath.trimmed().isEmpty() || url == nil)
            {
                return;
            }

            NSURL* previous = m_urls.value(localPath, nil);
            if (previous != nil)
            {
                [previous stopAccessingSecurityScopedResource];
                [previous release];
            }

            m_urls.insert(localPath, [url copy]);
        }

    private:
        QHash<QString, NSURL*> m_urls;
    };

    ScopedResourceRegistry& scopedResourceRegistry()
    {
        static ScopedResourceRegistry registry;
        return registry;
    }

    QString localizedErrorText(NSError* error, const QString& fallbackText)
    {
        if (error == nil || error.localizedDescription == nil)
        {
            return fallbackText;
        }

        return QString::fromUtf8([error.localizedDescription UTF8String]);
    }

    NSURL* resolvedFileUrlForPath(const QString& localPath)
    {
        const QString normalizedLocalPath = normalizeAbsolutePath(localPath);
        if (normalizedLocalPath.isEmpty())
        {
            return nil;
        }

        if (NSURL* retainedUrl = scopedResourceRegistry().urlForPath(normalizedLocalPath))
        {
            return retainedUrl;
        }

        return QUrl::fromLocalFile(normalizedLocalPath).toNSURL();
    }

    QString localPathFromNsUrl(NSURL* nsUrl)
    {
        if (nsUrl == nil)
        {
            return {};
        }

        const NSString* nsPath = [nsUrl path];
        if (nsPath == nil || [nsPath length] == 0)
        {
            return {};
        }

        return normalizeAbsolutePath(QString::fromUtf8([nsPath UTF8String]));
    }

    bool effectiveUrlForSource(
        const QUrl& url,
        const bool parentDirectoryScope,
        QUrl* effectiveUrl,
        QString* errorMessage)
    {
        if (!url.isValid())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("The selected iOS document URL is invalid.");
            }
            return false;
        }

        NSURL* nsUrl = url.toNSURL();
        if (nsUrl == nil)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to convert the selected iOS document URL into an NSURL.");
            }
            return false;
        }

        NSURL* nextEffectiveNsUrl = nsUrl;
        if (parentDirectoryScope)
        {
            nextEffectiveNsUrl = [nsUrl URLByDeletingLastPathComponent];
        }

        if (nextEffectiveNsUrl == nil)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("The effective iOS document URL is invalid.");
            }
            return false;
        }

        const QUrl nextEffectiveUrl = QUrl::fromNSURL(nextEffectiveNsUrl);
        if (!nextEffectiveUrl.isValid())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("The effective iOS document URL is invalid.");
            }
            return false;
        }

        if (effectiveUrl != nullptr)
        {
            *effectiveUrl = nextEffectiveUrl;
        }
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    bool beginAccessForUrl(const QUrl& url, QString* errorMessage)
    {
        NSURL* nsUrl = url.toNSURL();
        if (nsUrl == nil)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to convert the selected iOS document URL into an NSURL.");
            }
            return false;
        }

        const QString normalizedLocalPath = localPathFromNsUrl(nsUrl);
        if (normalizedLocalPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("The selected iOS document URL does not resolve to a local path.");
            }
            return false;
        }

        if (scopedResourceRegistry().contains(normalizedLocalPath))
        {
            return true;
        }

        const bool accessibleBeforeAccess = pathExists(normalizedLocalPath);
        const BOOL started = [nsUrl startAccessingSecurityScopedResource];
        if (started)
        {
            scopedResourceRegistry().retain(normalizedLocalPath, nsUrl);
        }

        const bool accessibleAfterAccess = pathExists(normalizedLocalPath);
        WhatSon::Debug::trace(
            QStringLiteral("ios.securityScoped"),
            QStringLiteral("beginAccess"),
            QStringLiteral("path=%1 started=%2 accessibleBefore=%3 accessibleAfter=%4")
                .arg(normalizedLocalPath)
                .arg(started ? QStringLiteral("1") : QStringLiteral("0"))
                .arg(accessibleBeforeAccess ? QStringLiteral("1") : QStringLiteral("0"))
                .arg(accessibleAfterAccess ? QStringLiteral("1") : QStringLiteral("0")));

        if (started || accessibleBeforeAccess || accessibleAfterAccess)
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral(
                "iOS denied access to the selected document provider URL. Re-select the WhatSon Hub from Files.");
        }
        return false;
    }
} // namespace

QString localPathForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage)
{
    QUrl effectiveUrl;
    if (!effectiveUrlForSource(url, parentDirectoryScope, &effectiveUrl, errorMessage))
    {
        return {};
    }

    NSURL* nsUrl = effectiveUrl.toNSURL();
    const QString normalizedLocalPath = localPathFromNsUrl(nsUrl);
    if (normalizedLocalPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("The selected iOS document URL does not resolve to a local path.");
        }
        return {};
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return normalizedLocalPath;
}

bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage)
{
    QUrl effectiveUrl;
    if (!effectiveUrlForSource(url, parentDirectoryScope, &effectiveUrl, errorMessage))
    {
        return false;
    }
    return beginAccessForUrl(effectiveUrl, errorMessage);
}

bool ensureAccessForPath(const QString& localPath, QString* errorMessage)
{
    const QString normalizedLocalPath = normalizeAbsolutePath(localPath);
    if (normalizedLocalPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("The resolved iOS hub path is empty.");
        }
        return false;
    }

    if (pathExists(normalizedLocalPath))
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    return beginAccessForUrl(QUrl::fromLocalFile(normalizedLocalPath), errorMessage);
}

QByteArray bookmarkDataForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage)
{
    QUrl effectiveUrl;
    if (!effectiveUrlForSource(url, parentDirectoryScope, &effectiveUrl, errorMessage))
    {
        return {};
    }

    NSURL* nsUrl = effectiveUrl.toNSURL();
    if (nsUrl == nil)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to convert the selected iOS document URL into an NSURL.");
        }
        return {};
    }

    NSError* bookmarkError = nil;
    NSData* bookmarkData = [nsUrl bookmarkDataWithOptions:0
                           includingResourceValuesForKeys:nil
                                            relativeToURL:nil
                                                    error:&bookmarkError];
    if (bookmarkData == nil)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = localizedErrorText(
                bookmarkError,
                QStringLiteral("Failed to create a persistent bookmark for the selected iOS WhatSon Hub."));
        }
        return {};
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return QByteArray(
        static_cast<const char*>([bookmarkData bytes]),
        static_cast<qsizetype>([bookmarkData length]));
}

bool restoreAccessFromBookmarkData(
    const QByteArray& bookmarkData,
    QString* restoredPath,
    QString* errorMessage)
{
    if (restoredPath != nullptr)
    {
        restoredPath->clear();
    }

    if (bookmarkData.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("The stored iOS WhatSon Hub bookmark is empty.");
        }
        return false;
    }

    NSData* bookmarkNsData = [NSData dataWithBytes:bookmarkData.constData()
                                            length:static_cast<NSUInteger>(bookmarkData.size())];
    if (bookmarkNsData == nil)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to materialize the stored iOS WhatSon Hub bookmark.");
        }
        return false;
    }

    BOOL staleBookmark = NO;
    NSError* restoreError = nil;
    NSURLBookmarkResolutionOptions options = NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting;
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140200
    options |= NSURLBookmarkResolutionWithoutImplicitStartAccessing;
#endif
    NSURL* resolvedUrl = [NSURL URLByResolvingBookmarkData:bookmarkNsData
                                                   options:options
                                             relativeToURL:nil
                                       bookmarkDataIsStale:&staleBookmark
                                                     error:&restoreError];
    if (resolvedUrl == nil)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = localizedErrorText(
                restoreError,
                QStringLiteral("Failed to restore the stored iOS WhatSon Hub bookmark."));
        }
        return false;
    }

    const QString resolvedLocalPath = normalizeAbsolutePath(
        QString::fromUtf8([[resolvedUrl path] UTF8String]));
    const BOOL started = [resolvedUrl startAccessingSecurityScopedResource];
    if (started)
    {
        scopedResourceRegistry().retain(resolvedLocalPath, resolvedUrl);
    }

    const bool accessibleAfterRestore =
        !resolvedLocalPath.trimmed().isEmpty() && pathExists(resolvedLocalPath);
    WhatSon::Debug::trace(
        QStringLiteral("ios.securityScoped"),
        QStringLiteral("restoreBookmark"),
        QStringLiteral("path=%1 started=%2 stale=%3 accessible=%4")
            .arg(resolvedLocalPath)
            .arg(started ? QStringLiteral("1") : QStringLiteral("0"))
            .arg(staleBookmark ? QStringLiteral("1") : QStringLiteral("0"))
            .arg(accessibleAfterRestore ? QStringLiteral("1") : QStringLiteral("0")));

    if (!started && !accessibleAfterRestore)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral(
                "iOS denied access while restoring the stored WhatSon Hub bookmark.");
        }
        return false;
    }

    if (restoredPath != nullptr)
    {
        *restoredPath = resolvedLocalPath;
    }
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}
#endif
} // namespace WhatSon::Apple::SecurityScopedResourceAccess
