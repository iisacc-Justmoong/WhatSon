#pragma once

#include <QByteArray>
#include <QEvent>
#include <QFileSystemWatcher>
#include <QObject>
#include <QPointer>
#include <QTimer>

#include <functional>

class QCoreApplication;

class WhatSonHubSyncController final : public QObject
{
    Q_OBJECT

public:
    explicit WhatSonHubSyncController(QObject* parent = nullptr);

    void attachToApplication(QCoreApplication* application);
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

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void handleApplicationStateChanged(Qt::ApplicationState state);
    void onWatchedPathChanged(const QString& path);
    void onPeriodicTimeout();
    void onDebounceTimeout();

private:
    [[nodiscard]] QByteArray computeHubSignature(const QString& hubPath) const;
    [[nodiscard]] QStringList collectWatchPaths(const QString& hubPath) const;
    [[nodiscard]] bool isInteractiveSyncEvent(QEvent::Type type) const;
    void scheduleSyncCheck();
    void refreshBaseline(bool rebuildWatcher);
    void rebuildWatcher();
    void clearWatcher();

    QPointer<QCoreApplication> m_application;
    QFileSystemWatcher m_fileSystemWatcher;
    QTimer m_periodicTimer;
    QTimer m_debounceTimer;
    std::function<bool(const QString&, QString*)> m_reloadCallback;
    QString m_currentHubPath;
    QByteArray m_lastKnownHubSignature;
    bool m_reloadInProgress = false;
    bool m_localMutationPending = false;
};
