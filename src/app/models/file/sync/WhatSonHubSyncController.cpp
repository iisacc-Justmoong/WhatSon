#include "app/models/file/sync/WhatSonHubSyncController.hpp"

#include "app/models/file/hub/WhatSonHubPathUtils.hpp"

#include <utility>

WhatSonHubSyncController::WhatSonHubSyncController(QObject* parent)
    : QObject(parent)
{
    QObject::connect(
        &m_scheduler,
        &WhatSonHubSyncScheduler::syncCheckDue,
        this,
        &WhatSonHubSyncController::onScheduledSyncCheck);
    QObject::connect(
        &m_watcher,
        &WhatSonHubSyncWatcher::watchedPathChanged,
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
        m_scheduler.stopPeriodic();
        m_lastKnownObservation = {};
        m_watcher.clear();
        return;
    }

    refreshBaseline(true);
    m_scheduler.startPeriodic();
}

QString WhatSonHubSyncController::currentHubPath() const
{
    return m_currentHubPath;
}

void WhatSonHubSyncController::setPeriodicIntervalMs(const int intervalMs)
{
    m_scheduler.setPeriodicIntervalMs(intervalMs);
}

void WhatSonHubSyncController::setDebounceIntervalMs(const int intervalMs)
{
    m_scheduler.setDebounceIntervalMs(intervalMs);
}

void WhatSonHubSyncController::requestSyncHint()
{
    if (m_currentHubPath.isEmpty())
    {
        return;
    }

    m_scheduler.requestSyncCheck();
}

void WhatSonHubSyncController::acknowledgeLocalMutation()
{
    if (m_currentHubPath.isEmpty())
    {
        return;
    }

    m_localMutationPending = true;
    m_scheduler.requestSyncCheck();
}

void WhatSonHubSyncController::onWatchedPathChanged(const QString& path)
{
    Q_UNUSED(path);
    m_scheduler.requestSyncCheck();
}

void WhatSonHubSyncController::onScheduledSyncCheck()
{
    if (m_reloadInProgress || m_currentHubPath.isEmpty())
    {
        return;
    }

    const WhatSonHubSyncObservation currentObservation = m_observationBuilder.inspectHub(m_currentHubPath);
    if (currentObservation.signature == m_lastKnownObservation.signature)
    {
        m_localMutationPending = false;
        m_lastKnownObservation = currentObservation;
        m_watcher.applyDirectoryWatchPaths(m_lastKnownObservation.directoryWatchPaths);
        return;
    }

    if (m_localMutationPending)
    {
        m_localMutationPending = false;
        m_lastKnownObservation = currentObservation;
        m_watcher.applyDirectoryWatchPaths(m_lastKnownObservation.directoryWatchPaths);
        return;
    }

    if (!m_reloadCallback)
    {
        m_lastKnownObservation = currentObservation;
        m_watcher.applyDirectoryWatchPaths(m_lastKnownObservation.directoryWatchPaths);
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

    m_lastKnownObservation = currentObservation;
    m_watcher.applyDirectoryWatchPaths(m_lastKnownObservation.directoryWatchPaths);
    emit syncReloaded(m_currentHubPath);
}

void WhatSonHubSyncController::refreshBaseline(const bool rebuildWatcher)
{
    if (m_currentHubPath.isEmpty())
    {
        m_lastKnownObservation = {};
        m_watcher.clear();
        return;
    }

    m_lastKnownObservation = m_observationBuilder.inspectHub(m_currentHubPath);
    if (rebuildWatcher)
    {
        m_watcher.applyDirectoryWatchPaths(m_lastKnownObservation.directoryWatchPaths);
    }
}
