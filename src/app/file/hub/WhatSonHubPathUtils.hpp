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

    inline QString androidDocumentIdFromUrl(const QUrl& url)
    {
        const QString encodedUrl = url.toString(QUrl::FullyEncoded);
        for (const QString& marker : {QStringLiteral("/document/"), QStringLiteral("/tree/")})
        {
            const int markerIndex = encodedUrl.indexOf(marker);
            if (markerIndex < 0)
            {
                continue;
            }

            const int valueStart = markerIndex + marker.size();
            const int valueEnd = encodedUrl.indexOf(QLatin1Char('/'), valueStart);
            const QString encodedDocumentId = valueEnd < 0
                                                  ? encodedUrl.mid(valueStart)
                                                  : encodedUrl.mid(valueStart, valueEnd - valueStart);
            if (!encodedDocumentId.isEmpty())
            {
                return QUrl::fromPercentEncoding(encodedDocumentId.toUtf8());
            }
        }

        return {};
    }

    inline QString androidExternalStoragePathFromDocumentId(const QString& documentId)
    {
        const int separatorIndex = documentId.indexOf(QLatin1Char(':'));
        const QString volumeName = separatorIndex < 0 ? documentId : documentId.left(separatorIndex).trimmed();
        QString relativePath = separatorIndex < 0 ? QString() : documentId.mid(separatorIndex + 1).trimmed();

        if (volumeName.isEmpty())
        {
            return {};
        }

        if (volumeName.compare(QStringLiteral("raw"), Qt::CaseInsensitive) == 0)
        {
            return relativePath.trimmed().isEmpty() ? QString() : QDir::cleanPath(relativePath);
        }

        QString basePath;
        if (volumeName.compare(QStringLiteral("primary"), Qt::CaseInsensitive) == 0)
        {
            basePath = QStringLiteral("/storage/emulated/0");
        }
        else if (volumeName.compare(QStringLiteral("home"), Qt::CaseInsensitive) == 0)
        {
            basePath = QStringLiteral("/storage/emulated/0/Documents");
            if (relativePath.startsWith(QStringLiteral("Documents/"), Qt::CaseInsensitive))
            {
                relativePath.remove(0, QStringLiteral("Documents/").size());
            }
        }
        else
        {
            basePath = QStringLiteral("/storage/%1").arg(volumeName);
        }

        if (relativePath.isEmpty())
        {
            return QDir::cleanPath(basePath);
        }

        return QDir::cleanPath(QDir(basePath).filePath(relativePath));
    }

    inline QString androidDownloadsPathFromDocumentId(const QString& documentId)
    {
        const QString trimmedDocumentId = documentId.trimmed();
        if (trimmedDocumentId.isEmpty())
        {
            return {};
        }

        if (trimmedDocumentId.startsWith(QStringLiteral("raw:"), Qt::CaseInsensitive))
        {
            return QDir::cleanPath(trimmedDocumentId.mid(4));
        }

        if (trimmedDocumentId.startsWith(QStringLiteral("/storage/"), Qt::CaseInsensitive))
        {
            return QDir::cleanPath(trimmedDocumentId);
        }

        if (trimmedDocumentId.startsWith(QStringLiteral("file://"), Qt::CaseInsensitive))
        {
            return QDir::cleanPath(QUrl(trimmedDocumentId).toLocalFile());
        }

        const QString basePath = QStringLiteral("/storage/emulated/0/Download");
        const int separatorIndex = trimmedDocumentId.indexOf(QLatin1Char(':'));
        const QString rootName = separatorIndex < 0 ? trimmedDocumentId : trimmedDocumentId.left(separatorIndex).trimmed();
        QString relativePath = separatorIndex < 0 ? QString() : trimmedDocumentId.mid(separatorIndex + 1).trimmed();

        if (rootName.compare(QStringLiteral("download"), Qt::CaseInsensitive) != 0
            && rootName.compare(QStringLiteral("downloads"), Qt::CaseInsensitive) != 0)
        {
            return {};
        }

        if (relativePath.startsWith(QStringLiteral("Download/"), Qt::CaseInsensitive))
        {
            relativePath.remove(0, QStringLiteral("Download/").size());
        }

        if (relativePath.isEmpty())
        {
            return basePath;
        }

        return QDir::cleanPath(QDir(basePath).filePath(relativePath));
    }

    inline QString resolveAndroidDocumentPath(const QUrl& url)
    {
        if (!url.isValid() || url.scheme() != QStringLiteral("content"))
        {
            return {};
        }

        const QString authority = url.host().trimmed();
        const QString documentId = androidDocumentIdFromUrl(url);
        if (documentId.isEmpty())
        {
            return {};
        }

        if (authority == QStringLiteral("com.android.externalstorage.documents"))
        {
            return androidExternalStoragePathFromDocumentId(documentId);
        }

        if (authority == QStringLiteral("com.android.providers.downloads.documents"))
        {
            return androidDownloadsPathFromDocumentId(documentId);
        }

        return {};
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
            const QString resolvedAndroidPath = resolveAndroidDocumentPath(QUrl(trimmed));
            if (!resolvedAndroidPath.isEmpty())
            {
                return QDir::cleanPath(resolvedAndroidPath);
            }
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
            const QString resolvedAndroidPath = resolveAndroidDocumentPath(QUrl(trimmed));
            if (!resolvedAndroidPath.isEmpty())
            {
                return QDir::cleanPath(QFileInfo(resolvedAndroidPath).absoluteFilePath());
            }
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

        const QString resolvedAndroidPath = resolveAndroidDocumentPath(url);
        if (!resolvedAndroidPath.isEmpty())
        {
            return normalizeAbsolutePath(resolvedAndroidPath);
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
