#pragma once

#include <QByteArray>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>

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
    void onPeriodicTimeout();
    void onDebounceTimeout();

private:
    struct HubObservation final
    {
        QByteArray signature;
        QStringList watchPaths;
    };

    [[nodiscard]] HubObservation inspectHub(const QString& hubPath) const;
    void scheduleSyncCheck();
    void refreshBaseline(bool rebuildWatcher);
    void rebuildWatcher();
    void rebuildWatcher(QStringList watchPaths);
    void clearWatcher();

    QFileSystemWatcher m_fileSystemWatcher;
    QTimer m_periodicTimer;
    QTimer m_debounceTimer;
    std::function<bool(const QString&, QString*)> m_reloadCallback;
    QString m_currentHubPath;
    QByteArray m_lastKnownHubSignature;
    QStringList m_lastObservedWatchPaths;
    QStringList m_appliedWatchPaths;
    bool m_reloadInProgress = false;
    bool m_localMutationPending = false;
};
