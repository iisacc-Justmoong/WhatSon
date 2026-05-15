#include "app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

#include <algorithm>
#include <utility>

namespace
{
    QString signatureRecordForInfo(const QString& relativePath, const QFileInfo& info)
    {
        return QStringLiteral("%1|%2|%3|%4")
            .arg(relativePath,
                 info.isDir() ? QStringLiteral("dir") : QStringLiteral("file"),
                 QString::number(info.size()),
                 QString::number(info.lastModified().toMSecsSinceEpoch()));
    }

    QString normalizeObservedRelativePath(QString relativePath)
    {
        relativePath = QDir::cleanPath(relativePath.trimmed());
        relativePath.replace(QLatin1Char('\\'), QLatin1Char('/'));
        return relativePath;
    }

    bool shouldIgnoreObservedRelativePath(const QString& relativePath)
    {
        const QString normalizedRelativePath = normalizeObservedRelativePath(relativePath);
        return normalizedRelativePath == QStringLiteral(".whatson")
            || normalizedRelativePath.startsWith(QStringLiteral(".whatson/"));
    }

    QStringList normalizeDirectoryWatchPaths(QStringList watchPaths)
    {
        QStringList normalized;
        normalized.reserve(watchPaths.size());
        for (QString path : watchPaths)
        {
            path = QDir::cleanPath(path.trimmed());
            if (path.isEmpty())
            {
                continue;
            }
            normalized.push_back(std::move(path));
        }
        normalized.removeDuplicates();
        std::sort(normalized.begin(), normalized.end());
        return normalized;
    }
} // namespace

WhatSonHubSyncObservation WhatSonHubSyncObservationBuilder::inspectHub(const QString& hubPath) const
{
    WhatSonHubSyncObservation observation;
    const QFileInfo rootInfo(hubPath);
    if (!rootInfo.exists() || !rootInfo.isDir())
    {
        return observation;
    }

    QStringList signatureRecords;
    signatureRecords.reserve(64);
    signatureRecords.push_back(signatureRecordForInfo(QStringLiteral("."), rootInfo));
    observation.directoryWatchPaths.push_back(rootInfo.absoluteFilePath());

    QDirIterator iterator(
        hubPath,
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
        QDirIterator::Subdirectories);
    const QDir rootDirectory(hubPath);
    while (iterator.hasNext())
    {
        iterator.next();
        const QFileInfo info = iterator.fileInfo();
        if (!info.exists())
        {
            continue;
        }

        const QString relativePath = rootDirectory.relativeFilePath(info.absoluteFilePath());
        if (shouldIgnoreObservedRelativePath(relativePath))
        {
            continue;
        }
        signatureRecords.push_back(signatureRecordForInfo(relativePath, info));
        if (info.isDir())
        {
            observation.directoryWatchPaths.push_back(info.absoluteFilePath());
        }
    }

    std::sort(signatureRecords.begin(), signatureRecords.end());
    QCryptographicHash hash(QCryptographicHash::Sha256);
    for (const QString& record : signatureRecords)
    {
        hash.addData(record.toUtf8());
        hash.addData(QByteArrayView("\n", 1));
    }
    observation.signature = hash.result();
    observation.directoryWatchPaths = normalizeDirectoryWatchPaths(std::move(observation.directoryWatchPaths));
    return observation;
}
