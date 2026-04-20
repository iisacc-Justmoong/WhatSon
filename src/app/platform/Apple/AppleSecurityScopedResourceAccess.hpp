#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace WhatSon::Apple::SecurityScopedResourceAccess
{
#if defined(Q_OS_IOS)
    QUrl scopedUrlForUrl(const QUrl& url, int ancestorDepth, QString* errorMessage);
    QString localPathForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    QString localPathForUrl(const QUrl& url, int ancestorDepth, QString* errorMessage);
    bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    bool startAccessForUrl(const QUrl& url, int ancestorDepth, QString* errorMessage);
    bool ensureAccessForPath(const QString& localPath, QString* errorMessage);
    QByteArray bookmarkDataForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    QByteArray bookmarkDataForUrl(const QUrl& url, int ancestorDepth, QString* errorMessage);
    bool restoreAccessFromBookmarkData(
        const QByteArray& bookmarkData,
        QString* restoredPath,
        QString* errorMessage);
#else
    inline QUrl scopedUrlForUrl(const QUrl&, int, QString*)
    {
        return {};
    }

    inline QString localPathForUrl(const QUrl&, bool, QString*)
    {
        return {};
    }

    inline QString localPathForUrl(const QUrl&, int, QString*)
    {
        return {};
    }

    inline bool startAccessForUrl(const QUrl&, bool, QString*)
    {
        return true;
    }

    inline bool startAccessForUrl(const QUrl&, int, QString*)
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

    inline QByteArray bookmarkDataForUrl(const QUrl&, int, QString*)
    {
        return {};
    }

    inline bool restoreAccessFromBookmarkData(const QByteArray&, QString*, QString*)
    {
        return false;
    }
#endif
} // namespace WhatSon::Apple::SecurityScopedResourceAccess
