#pragma once

#include <QObject>
#include <QTimer>

class WhatSonHubSyncScheduler final : public QObject
{
    Q_OBJECT

public:
    explicit WhatSonHubSyncScheduler(QObject* parent = nullptr);

    void setPeriodicIntervalMs(int intervalMs);
    void setDebounceIntervalMs(int intervalMs);
    void startPeriodic();
    void stopPeriodic();
    void requestSyncCheck();

signals:
    void syncCheckDue();

private slots:
    void emitSyncCheckDue();

private:
    QTimer m_periodicTimer;
    QTimer m_debounceTimer;
};
