#pragma once

#include "app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp"
#include "app/models/file/sync/WhatSonHubSyncScheduler.hpp"
#include "app/models/file/sync/WhatSonHubSyncWatcher.hpp"

#include <QObject>

#include <functional>

class WhatSonHubSyncController final : public QObject
{
    Q_OBJECT

public:
    explicit WhatSonHubSyncController(QObject* parent = nullptr);

    void setReloadCallback(std::function<bool(const QString& hubPath, QString* errorMessage)> callback);
    void setCurrentHubPath(const QString& hubPath);
    [[nodiscard]] QString currentHubPath() const;
    void setPeriodicIntervalMs(int intervalMs);
    void setDebounceIntervalMs(int intervalMs);

public slots:
    void requestSyncHint();
    void acknowledgeLocalMutation();

signals:
    void syncReloaded(const QString& hubPath);
    void syncFailed(const QString& errorMessage);

private slots:
    void onWatchedPathChanged(const QString& path);
    void onScheduledSyncCheck();

private:
    void refreshBaseline(bool rebuildWatcher);

    WhatSonHubSyncObservationBuilder m_observationBuilder;
    WhatSonHubSyncScheduler m_scheduler;
    WhatSonHubSyncWatcher m_watcher;
    std::function<bool(const QString&, QString*)> m_reloadCallback;
    QString m_currentHubPath;
    WhatSonHubSyncObservation m_lastKnownObservation;
    bool m_reloadInProgress = false;
    bool m_localMutationPending = false;
};
