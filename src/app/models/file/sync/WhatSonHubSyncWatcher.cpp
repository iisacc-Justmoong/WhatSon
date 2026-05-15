#include "app/models/file/sync/WhatSonHubSyncWatcher.hpp"

#include <QDir>
#include <QSet>

#include <algorithm>
#include <utility>

namespace
{
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

WhatSonHubSyncWatcher::WhatSonHubSyncWatcher(QObject* parent)
    : QObject(parent)
{
    QObject::connect(
        &m_fileSystemWatcher,
        &QFileSystemWatcher::directoryChanged,
        this,
        &WhatSonHubSyncWatcher::watchedPathChanged);
}

void WhatSonHubSyncWatcher::applyDirectoryWatchPaths(QStringList watchPaths)
{
    watchPaths = normalizeDirectoryWatchPaths(std::move(watchPaths));
    if (watchPaths == m_appliedDirectoryWatchPaths)
    {
        return;
    }

    const QSet<QString> nextPathSet(watchPaths.begin(), watchPaths.end());
    const QSet<QString> currentPathSet(
        m_appliedDirectoryWatchPaths.begin(),
        m_appliedDirectoryWatchPaths.end());

    QStringList pathsToRemove;
    pathsToRemove.reserve(m_appliedDirectoryWatchPaths.size());
    for (const QString& existingPath : m_appliedDirectoryWatchPaths)
    {
        if (!nextPathSet.contains(existingPath))
        {
            pathsToRemove.push_back(existingPath);
        }
    }

    QStringList pathsToAdd;
    pathsToAdd.reserve(watchPaths.size());
    for (const QString& candidatePath : watchPaths)
    {
        if (!currentPathSet.contains(candidatePath))
        {
            pathsToAdd.push_back(candidatePath);
        }
    }

    if (!pathsToRemove.isEmpty())
    {
        m_fileSystemWatcher.removePaths(pathsToRemove);
    }
    if (!pathsToAdd.isEmpty())
    {
        m_fileSystemWatcher.addPaths(pathsToAdd);
    }
    m_appliedDirectoryWatchPaths = std::move(watchPaths);
}

void WhatSonHubSyncWatcher::clear()
{
    const QStringList filePaths = m_fileSystemWatcher.files();
    if (!filePaths.isEmpty())
    {
        m_fileSystemWatcher.removePaths(filePaths);
    }

    const QStringList directoryPaths = m_fileSystemWatcher.directories();
    if (!directoryPaths.isEmpty())
    {
        m_fileSystemWatcher.removePaths(directoryPaths);
    }

    m_appliedDirectoryWatchPaths.clear();
}

QStringList WhatSonHubSyncWatcher::appliedDirectoryWatchPaths() const
{
    return m_appliedDirectoryWatchPaths;
}
