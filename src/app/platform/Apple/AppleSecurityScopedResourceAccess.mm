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

    bool beginAccessForLocalFileUrl(const QUrl& url, QString* errorMessage)
    {
        const QString normalizedLocalPath = normalizeAbsolutePath(url.toLocalFile());
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

        NSURL* nsUrl = url.toNSURL();
        if (nsUrl == nil)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to convert the selected iOS document URL into an NSURL.");
            }
            return false;
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

bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage)
{
    if (!url.isValid())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("The selected iOS document URL is invalid.");
        }
        return false;
    }

    if (!url.isLocalFile())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("The selected iOS document URL is not a local filesystem URL.");
        }
        return false;
    }

    QUrl effectiveUrl = url;
    if (parentDirectoryScope)
    {
        const QString parentDirectoryPath = QFileInfo(url.toLocalFile()).absolutePath();
        effectiveUrl = QUrl::fromLocalFile(parentDirectoryPath);
    }

    return beginAccessForLocalFileUrl(effectiveUrl, errorMessage);
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

    return beginAccessForLocalFileUrl(QUrl::fromLocalFile(normalizedLocalPath), errorMessage);
}
#endif
} // namespace WhatSon::Apple::SecurityScopedResourceAccess
