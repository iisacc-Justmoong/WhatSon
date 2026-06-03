#pragma once

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QUrl>

namespace WhatSon::HubPath
{
    inline bool isNonLocalUrl(const QString& path)
    {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty())
        {
            return false;
        }

        const QUrl url(trimmed);
        return url.isValid() && !url.scheme().isEmpty() && !url.isLocalFile();
    }

    inline QString normalizePath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        if (isNonLocalUrl(trimmed))
        {
            return trimmed;
        }

        return QDir::cleanPath(trimmed);
    }

    inline QString normalizeAbsolutePath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        if (isNonLocalUrl(trimmed))
        {
            return trimmed;
        }

        return QDir::cleanPath(QFileInfo(trimmed).absoluteFilePath());
    }

    inline QString joinPath(const QString& basePath, const QString& relativePath)
    {
        const QString normalizedBasePath = normalizePath(basePath);
        const QString trimmedRelativePath = relativePath.trimmed();

        if (normalizedBasePath.isEmpty())
        {
            return normalizePath(trimmedRelativePath);
        }
        if (trimmedRelativePath.isEmpty())
        {
            return normalizedBasePath;
        }

        if (!isNonLocalUrl(normalizedBasePath))
        {
            return QDir::cleanPath(QDir(normalizedBasePath).filePath(trimmedRelativePath));
        }

        QString joinedPath = normalizedBasePath;
        if (joinedPath.endsWith(QLatin1Char('/')))
        {
            joinedPath.chop(1);
        }

        const QStringList segments = trimmedRelativePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        for (const QString& segment : segments)
        {
            joinedPath += QLatin1Char('/');
            joinedPath += QString::fromUtf8(QUrl::toPercentEncoding(segment));
        }

        return joinedPath;
    }

    inline QString parentPath(const QString& path)
    {
        const QString normalizedPath = normalizePath(path);
        if (normalizedPath.isEmpty())
        {
            return {};
        }

        if (!isNonLocalUrl(normalizedPath))
        {
            return QFileInfo(normalizedPath).absolutePath();
        }

        QUrl url(normalizedPath);
        QString uriPath = url.path();
        const int lastSlashIndex = uriPath.lastIndexOf(QLatin1Char('/'));
        if (lastSlashIndex <= 0)
        {
            return {};
        }

        uriPath = uriPath.left(lastSlashIndex);
        url.setPath(uriPath);
        return url.toString(QUrl::FullyEncoded);
    }

    inline QString pathFromUrl(const QUrl& url)
    {
        if (!url.isValid())
        {
            return {};
        }

        if (url.isLocalFile())
        {
            return normalizeAbsolutePath(url.toLocalFile());
        }

        return normalizePath(url.toString(QUrl::FullyEncoded));
    }

    inline QUrl urlFromPath(const QString& path)
    {
        const QString normalizedPath = normalizePath(path);
        if (normalizedPath.isEmpty())
        {
            return {};
        }

        if (isNonLocalUrl(normalizedPath))
        {
            return QUrl(normalizedPath);
        }

        return QUrl::fromLocalFile(normalizedPath);
    }
} // namespace WhatSon::HubPath
