#include "WhatSonHubSyncController.hpp"

#include "file/hub/WhatSonHubPathUtils.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStringList>

#include <algorithm>

namespace
{
    constexpr int kDefaultPeriodicIntervalMs = 5000;
    constexpr int kDefaultDebounceIntervalMs = 350;

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
        return normalizedRelativePath == QStringLiteral(".whatson");
    }

    QStringList normalizeWatchPaths(QStringList watchPaths)
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
}

WhatSonHubSyncController::WhatSonHubSyncController(QObject* parent)
    : QObject(parent)
{
    m_periodicTimer.setInterval(kDefaultPeriodicIntervalMs);
    QObject::connect(&m_periodicTimer, &QTimer::timeout, this, &WhatSonHubSyncController::onPeriodicTimeout);

    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(kDefaultDebounceIntervalMs);
    QObject::connect(&m_debounceTimer, &QTimer::timeout, this, &WhatSonHubSyncController::onDebounceTimeout);

    QObject::connect(
        &m_fileSystemWatcher,
        &QFileSystemWatcher::directoryChanged,
        this,
        &WhatSonHubSyncController::onWatchedPathChanged);
    QObject::connect(
        &m_fileSystemWatcher,
        &QFileSystemWatcher::fileChanged,
        this,
        &WhatSonHubSyncController::onWatchedPathChanged);
}

void WhatSonHubSyncController::setReloadCallback(std::function<bool(const QString&, QString*)> callback)
{
    m_reloadCallback = std::move(callback);
}

void WhatSonHubSyncController::setCurrentHubPath(const QString& hubPath)
{
    const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                          ? QString()
                                          : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (m_currentHubPath == normalizedHubPath)
    {
        refreshBaseline(true);
        return;
    }

    m_currentHubPath = normalizedHubPath;
    m_localMutationPending = false;
    if (m_currentHubPath.isEmpty())
    {
        m_periodicTimer.stop();
        m_lastKnownHubSignature.clear();
        m_lastObservedWatchPaths.clear();
        clearWatcher();
        return;
    }

    refreshBaseline(true);
    if (!m_periodicTimer.isActive())
    {
        m_periodicTimer.start();
    }
}

QString WhatSonHubSyncController::currentHubPath() const
{
    return m_currentHubPath;
}

void WhatSonHubSyncController::setPeriodicIntervalMs(const int intervalMs)
{
    m_periodicTimer.setInterval(std::max(0, intervalMs));
}

void WhatSonHubSyncController::setDebounceIntervalMs(const int intervalMs)
{
    m_debounceTimer.setInterval(std::max(0, intervalMs));
}

void WhatSonHubSyncController::requestSyncHint()
{
    if (m_currentHubPath.isEmpty())
    {
        return;
    }

    scheduleSyncCheck();
}

void WhatSonHubSyncController::acknowledgeLocalMutation()
{
    if (m_currentHubPath.isEmpty())
    {
        return;
    }

    m_localMutationPending = true;
    scheduleSyncCheck();
}

void WhatSonHubSyncController::onWatchedPathChanged(const QString& path)
{
    Q_UNUSED(path);
    scheduleSyncCheck();
}

void WhatSonHubSyncController::onPeriodicTimeout()
{
    requestSyncHint();
}

void WhatSonHubSyncController::onDebounceTimeout()
{
    if (m_reloadInProgress || m_currentHubPath.isEmpty())
    {
        return;
    }

    const HubObservation currentObservation = inspectHub(m_currentHubPath);
    if (currentObservation.signature == m_lastKnownHubSignature)
    {
        m_localMutationPending = false;
        m_lastObservedWatchPaths = currentObservation.watchPaths;
        rebuildWatcher(m_lastObservedWatchPaths);
        return;
    }

    if (m_localMutationPending)
    {
        m_localMutationPending = false;
        m_lastKnownHubSignature = currentObservation.signature;
        m_lastObservedWatchPaths = currentObservation.watchPaths;
        rebuildWatcher(m_lastObservedWatchPaths);
        return;
    }

    if (!m_reloadCallback)
    {
        m_lastKnownHubSignature = currentObservation.signature;
        m_lastObservedWatchPaths = currentObservation.watchPaths;
        rebuildWatcher(m_lastObservedWatchPaths);
        return;
    }

    QString reloadError;
    m_reloadInProgress = true;
    const bool reloadSucceeded = m_reloadCallback(m_currentHubPath, &reloadError);
    m_reloadInProgress = false;

    if (!reloadSucceeded)
    {
        emit syncFailed(reloadError.trimmed().isEmpty()
                            ? QStringLiteral("Failed to synchronize the mounted WhatSon Hub.")
                            : reloadError.trimmed());
        return;
    }

    m_lastKnownHubSignature = currentObservation.signature;
    m_lastObservedWatchPaths = currentObservation.watchPaths;
    rebuildWatcher(m_lastObservedWatchPaths);
    emit syncReloaded(m_currentHubPath);
}

WhatSonHubSyncController::HubObservation WhatSonHubSyncController::inspectHub(const QString& hubPath) const
{
    HubObservation observation;
    const QFileInfo rootInfo(hubPath);
    if (!rootInfo.exists() || !rootInfo.isDir())
    {
        return observation;
    }

    QStringList signatureRecords;
    signatureRecords.reserve(64);
    signatureRecords.push_back(signatureRecordForInfo(QStringLiteral("."), rootInfo));
    observation.watchPaths.push_back(rootInfo.absoluteFilePath());

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
        observation.watchPaths.push_back(info.absoluteFilePath());
    }

    std::sort(signatureRecords.begin(), signatureRecords.end());
    QCryptographicHash hash(QCryptographicHash::Sha256);
    for (const QString& record : signatureRecords)
    {
        hash.addData(record.toUtf8());
        hash.addData(QByteArrayView("\n", 1));
    }
    observation.signature = hash.result();
    observation.watchPaths.removeDuplicates();
    return observation;
}

void WhatSonHubSyncController::scheduleSyncCheck()
{
    if (m_debounceTimer.interval() <= 0)
    {
        QTimer::singleShot(0, this, &WhatSonHubSyncController::onDebounceTimeout);
        return;
    }

    m_debounceTimer.start();
}

void WhatSonHubSyncController::refreshBaseline(const bool rebuildWatcherRequested)
{
    if (m_currentHubPath.isEmpty())
    {
        m_lastKnownHubSignature.clear();
        clearWatcher();
        return;
    }

    const HubObservation observation = inspectHub(m_currentHubPath);
    m_lastKnownHubSignature = observation.signature;
    m_lastObservedWatchPaths = observation.watchPaths;
    if (rebuildWatcherRequested)
    {
        rebuildWatcher(m_lastObservedWatchPaths);
    }
}

void WhatSonHubSyncController::rebuildWatcher()
{
    rebuildWatcher(m_lastObservedWatchPaths);
}

void WhatSonHubSyncController::rebuildWatcher(QStringList watchPaths)
{
    watchPaths = normalizeWatchPaths(std::move(watchPaths));
    if (watchPaths == m_appliedWatchPaths)
    {
        return;
    }

    clearWatcher();
    if (!watchPaths.isEmpty())
    {
        m_fileSystemWatcher.addPaths(watchPaths);
    }
    m_appliedWatchPaths = std::move(watchPaths);
}

void WhatSonHubSyncController::clearWatcher()
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

    m_appliedWatchPaths.clear();
}
