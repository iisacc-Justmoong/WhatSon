#pragma once

#include <QString>
#include <QUrl>

namespace WhatSon::Apple::SecurityScopedResourceAccess
{
#if defined(Q_OS_IOS)
    bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);
    bool ensureAccessForPath(const QString& localPath, QString* errorMessage);
#else
    inline bool startAccessForUrl(const QUrl&, bool, QString*)
    {
        return true;
    }

    inline bool ensureAccessForPath(const QString&, QString*)
    {
        return true;
    }
#endif
} // namespace WhatSon::Apple::SecurityScopedResourceAccess
