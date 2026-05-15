#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QStringList>

class WhatSonHubSyncWatcher final : public QObject
{
    Q_OBJECT

public:
    explicit WhatSonHubSyncWatcher(QObject* parent = nullptr);

    void applyDirectoryWatchPaths(QStringList watchPaths);
    void clear();
    [[nodiscard]] QStringList appliedDirectoryWatchPaths() const;

signals:
    void watchedPathChanged(const QString& path);

private:
    QFileSystemWatcher m_fileSystemWatcher;
    QStringList m_appliedDirectoryWatchPaths;
};
