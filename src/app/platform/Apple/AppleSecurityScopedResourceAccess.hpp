#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace WhatSon::Apple::SecurityScopedResourceAccess
{
#if defined(Q_OS_IOS)
    QString localPathForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    bool ensureAccessForPath(const QString& localPath, QString* errorMessage);
    QByteArray bookmarkDataForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    bool restoreAccessFromBookmarkData(
        const QByteArray& bookmarkData,
        QString* restoredPath,
        QString* errorMessage);
#else
    inline QString localPathForUrl(const QUrl&, bool, QString*)
    {
        return {};
    }

    inline bool startAccessForUrl(const QUrl&, bool, QString*)
    {
        return true;
    }

    inline bool ensureAccessForPath(const QString&, QString*)
    {
        return true;
    }

    inline QByteArray bookmarkDataForUrl(const QUrl&, bool, QString*)
    {
        return {};
    }

    inline bool restoreAccessFromBookmarkData(const QByteArray&, QString*, QString*)
    {
        return false;
    }
#endif
} // namespace WhatSon::Apple::SecurityScopedResourceAccess
