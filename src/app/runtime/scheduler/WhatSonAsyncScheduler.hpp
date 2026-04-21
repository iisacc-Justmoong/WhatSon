#pragma once

#include "app/runtime/scheduler/WhatSonCronExpression.hpp"

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class WhatSonAsyncScheduler final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int scheduleCount READ scheduleCount NOTIFY schedulesChanged)
    Q_PROPERTY(qint64 lastTickUnixSeconds READ lastTickUnixSeconds NOTIFY ticked)
    Q_PROPERTY(int hookRequestCount READ hookRequestCount NOTIFY hookRequestCountChanged)

public:
    explicit WhatSonAsyncScheduler(QObject* parent = nullptr);
    ~WhatSonAsyncScheduler() override;

    bool running() const noexcept;
    int scheduleCount() const noexcept;
    qint64 lastTickUnixSeconds() const noexcept;
    int hookRequestCount() const noexcept;

    Q_INVOKABLE qint64 unixNow() const;
    Q_INVOKABLE QVariantMap analyzeUnixTime(qint64 unixSeconds) const;
    Q_INVOKABLE QVariantList schedules() const;

    Q_INVOKABLE bool hookCronEvent(
        const QString& eventName,
        const QString& cronExpression,
        const QVariantMap& payload = {});
    Q_INVOKABLE bool hookIntervalEvent(
        const QString& eventName,
        qint64 intervalSeconds,
        const QVariantMap& payload = {});
    Q_INVOKABLE bool unhookEvent(const QString& eventName);
    Q_INVOKABLE void clearSchedules();

    Q_INVOKABLE int evaluateSchedulesAtUnixSeconds(qint64 unixSeconds);

    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();

public
    slots  :



    void requestViewModelHook();

    signals  :


    void runningChanged();
    void schedulesChanged();
    void ticked(qint64 unixSeconds);
    void scheduleTriggered(
        const QString& eventName,
        const QString& scheduleType,
        const QString& scheduleSpec,
        qint64 unixSeconds,
        const QVariantMap& unixAnalysis,
        const QVariantMap& payload);
    void schedulerWarning(const QString& message);
    void hookRequestCountChanged();
    void viewModelHookRequested();

private
    slots  :



    void onTickTimeout();

private:
    struct CronSchedule final
    {
        QString eventName;
        QString cronExpression;
        WhatSonCronExpression matcher;
        QVariantMap payload;
        qint64 lastTriggeredMinute = 0;
        bool hasTriggered = false;
    };

    struct IntervalSchedule final
    {
        QString eventName;
        qint64 intervalSeconds = 0;
        qint64 nextTriggerUnixSeconds = 0;
        qint64 lastTriggeredUnixSeconds = 0;
        bool hasTriggered = false;
        QVariantMap payload;
    };

    int findCronScheduleIndex(const QString& eventName) const;
    int findIntervalScheduleIndex(const QString& eventName) const;

    void emitScheduleTriggered(
        const QString& eventName,
        const QString& scheduleType,
        const QString& scheduleSpec,
        qint64 unixSeconds,
        const QVariantMap& payload);

    static QString normalizeEventName(const QString& eventName);

    QTimer m_tickTimer;
    QVector<CronSchedule> m_cronSchedules;
    QVector<IntervalSchedule> m_intervalSchedules;
    qint64 m_lastTickUnixSeconds = 0;
    int m_hookRequestCount = 0;
};
