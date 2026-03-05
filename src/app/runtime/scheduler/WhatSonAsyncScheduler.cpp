#include "WhatSonAsyncScheduler.hpp"

#include "WhatSonUnixTimeAnalyzer.hpp"

#include <QDateTime>
#include <QTimeZone>
#include <utility>

WhatSonAsyncScheduler::WhatSonAsyncScheduler(QObject* parent)
    : QObject(parent)
{
    m_tickTimer.setInterval(1000);
    m_tickTimer.setSingleShot(false);
    m_tickTimer.setTimerType(Qt::PreciseTimer);

    QObject::connect(&m_tickTimer, &QTimer::timeout, this, &WhatSonAsyncScheduler::onTickTimeout);
}

WhatSonAsyncScheduler::~WhatSonAsyncScheduler() = default;

bool WhatSonAsyncScheduler::running() const noexcept
{
    return m_tickTimer.isActive();
}

int WhatSonAsyncScheduler::scheduleCount() const noexcept
{
    return m_cronSchedules.size() + m_intervalSchedules.size();
}

qint64 WhatSonAsyncScheduler::lastTickUnixSeconds() const noexcept
{
    return m_lastTickUnixSeconds;
}

int WhatSonAsyncScheduler::hookRequestCount() const noexcept
{
    return m_hookRequestCount;
}

qint64 WhatSonAsyncScheduler::unixNow() const
{
    return WhatSon::Runtime::Scheduler::unixNowSeconds();
}

QVariantMap WhatSonAsyncScheduler::analyzeUnixTime(qint64 unixSeconds) const
{
    return WhatSon::Runtime::Scheduler::analyzeUnixSeconds(unixSeconds);
}

QVariantList WhatSonAsyncScheduler::schedules() const
{
    QVariantList result;
    result.reserve(scheduleCount());

    for (const CronSchedule& schedule : m_cronSchedules)
    {
        QVariantMap item{
            {QStringLiteral("type"), QStringLiteral("cron")},
            {QStringLiteral("eventName"), schedule.eventName},
            {QStringLiteral("scheduleSpec"), schedule.cronExpression},
            {QStringLiteral("payload"), schedule.payload},
            {QStringLiteral("hasTriggered"), schedule.hasTriggered},
            {QStringLiteral("lastTriggeredMinute"), schedule.lastTriggeredMinute}
        };
        result.push_back(item);
    }

    for (const IntervalSchedule& schedule : m_intervalSchedules)
    {
        QVariantMap item{
            {QStringLiteral("type"), QStringLiteral("interval")},
            {QStringLiteral("eventName"), schedule.eventName},
            {QStringLiteral("intervalSeconds"), schedule.intervalSeconds},
            {QStringLiteral("nextTriggerUnixSeconds"), schedule.nextTriggerUnixSeconds},
            {QStringLiteral("payload"), schedule.payload},
            {QStringLiteral("hasTriggered"), schedule.hasTriggered},
            {QStringLiteral("lastTriggeredUnixSeconds"), schedule.lastTriggeredUnixSeconds}
        };
        result.push_back(item);
    }

    return result;
}

bool WhatSonAsyncScheduler::hookCronEvent(
    const QString& eventName,
    const QString& cronExpression,
    const QVariantMap& payload)
{
    const QString normalizedName = normalizeEventName(eventName);
    if (normalizedName.isEmpty())
    {
        emit schedulerWarning(QStringLiteral("hookCronEvent rejected: eventName must not be empty."));
        return false;
    }

    WhatSonCronExpression matcher;
    QString parseError;
    if (!matcher.parse(cronExpression, &parseError))
    {
        emit schedulerWarning(
            QStringLiteral("hookCronEvent rejected for '%1': %2").arg(normalizedName, parseError));
        return false;
    }

    const int intervalIndex = findIntervalScheduleIndex(normalizedName);
    if (intervalIndex >= 0)
    {
        m_intervalSchedules.removeAt(intervalIndex);
    }

    CronSchedule schedule;
    schedule.eventName = normalizedName;
    schedule.cronExpression = matcher.expression();
    schedule.matcher = std::move(matcher);
    schedule.payload = payload;

    const int cronIndex = findCronScheduleIndex(normalizedName);
    if (cronIndex >= 0)
    {
        m_cronSchedules[cronIndex] = std::move(schedule);
    }
    else
    {
        m_cronSchedules.push_back(std::move(schedule));
    }

    emit schedulesChanged();
    return true;
}

bool WhatSonAsyncScheduler::hookIntervalEvent(
    const QString& eventName,
    qint64 intervalSeconds,
    const QVariantMap& payload)
{
    const QString normalizedName = normalizeEventName(eventName);
    if (normalizedName.isEmpty())
    {
        emit schedulerWarning(QStringLiteral("hookIntervalEvent rejected: eventName must not be empty."));
        return false;
    }

    if (intervalSeconds <= 0)
    {
        emit schedulerWarning(
            QStringLiteral("hookIntervalEvent rejected for '%1': intervalSeconds must be > 0.")
            .arg(normalizedName));
        return false;
    }

    const qint64 now = unixNow();

    const int cronIndex = findCronScheduleIndex(normalizedName);
    if (cronIndex >= 0)
    {
        m_cronSchedules.removeAt(cronIndex);
    }

    IntervalSchedule schedule;
    schedule.eventName = normalizedName;
    schedule.intervalSeconds = intervalSeconds;
    schedule.nextTriggerUnixSeconds = now + intervalSeconds;
    schedule.payload = payload;

    const int intervalIndex = findIntervalScheduleIndex(normalizedName);
    if (intervalIndex >= 0)
    {
        m_intervalSchedules[intervalIndex] = std::move(schedule);
    }
    else
    {
        m_intervalSchedules.push_back(std::move(schedule));
    }

    emit schedulesChanged();
    return true;
}

bool WhatSonAsyncScheduler::unhookEvent(const QString& eventName)
{
    const QString normalizedName = normalizeEventName(eventName);
    if (normalizedName.isEmpty())
    {
        return false;
    }

    bool removed = false;

    const int cronIndex = findCronScheduleIndex(normalizedName);
    if (cronIndex >= 0)
    {
        m_cronSchedules.removeAt(cronIndex);
        removed = true;
    }

    const int intervalIndex = findIntervalScheduleIndex(normalizedName);
    if (intervalIndex >= 0)
    {
        m_intervalSchedules.removeAt(intervalIndex);
        removed = true;
    }

    if (removed)
    {
        emit schedulesChanged();
    }

    return removed;
}

void WhatSonAsyncScheduler::clearSchedules()
{
    if (m_cronSchedules.isEmpty() && m_intervalSchedules.isEmpty())
    {
        return;
    }

    m_cronSchedules.clear();
    m_intervalSchedules.clear();
    emit schedulesChanged();
}

int WhatSonAsyncScheduler::evaluateSchedulesAtUnixSeconds(qint64 unixSeconds)
{
    if (unixSeconds < 0)
    {
        emit schedulerWarning(QStringLiteral("evaluateSchedulesAtUnixSeconds rejected: unixSeconds must be >= 0."));
        return 0;
    }

    if (m_lastTickUnixSeconds != unixSeconds)
    {
        m_lastTickUnixSeconds = unixSeconds;
        emit ticked(unixSeconds);
    }

    int triggeredCount = 0;

    const QDateTime localDateTime =
        QDateTime::fromSecsSinceEpoch(unixSeconds, QTimeZone::systemTimeZone());
    const qint64 unixMinute = unixSeconds / 60;
    for (CronSchedule& schedule : m_cronSchedules)
    {
        if (!schedule.matcher.matches(localDateTime))
        {
            continue;
        }
        if (schedule.hasTriggered && schedule.lastTriggeredMinute == unixMinute)
        {
            continue;
        }

        schedule.hasTriggered = true;
        schedule.lastTriggeredMinute = unixMinute;
        emitScheduleTriggered(
            schedule.eventName,
            QStringLiteral("cron"),
            schedule.cronExpression,
            unixSeconds,
            schedule.payload);
        ++triggeredCount;
    }

    for (IntervalSchedule& schedule : m_intervalSchedules)
    {
        if (unixSeconds < schedule.nextTriggerUnixSeconds)
        {
            continue;
        }

        schedule.hasTriggered = true;
        schedule.lastTriggeredUnixSeconds = unixSeconds;

        do
        {
            schedule.nextTriggerUnixSeconds += schedule.intervalSeconds;
        }
        while (schedule.nextTriggerUnixSeconds <= unixSeconds);

        emitScheduleTriggered(
            schedule.eventName,
            QStringLiteral("interval"),
            QString::number(schedule.intervalSeconds),
            unixSeconds,
            schedule.payload);
        ++triggeredCount;
    }

    return triggeredCount;
}

bool WhatSonAsyncScheduler::start()
{
    if (m_tickTimer.isActive())
    {
        return false;
    }

    m_tickTimer.start();
    emit runningChanged();
    return true;
}

void WhatSonAsyncScheduler::stop()
{
    if (!m_tickTimer.isActive())
    {
        return;
    }

    m_tickTimer.stop();
    emit runningChanged();
}

void WhatSonAsyncScheduler::requestViewModelHook()
{
    ++m_hookRequestCount;
    emit hookRequestCountChanged();
    emit viewModelHookRequested();
}

void WhatSonAsyncScheduler::onTickTimeout()
{
    evaluateSchedulesAtUnixSeconds(unixNow());
}

int WhatSonAsyncScheduler::findCronScheduleIndex(const QString& eventName) const
{
    for (int index = 0; index < m_cronSchedules.size(); ++index)
    {
        if (m_cronSchedules.at(index).eventName == eventName)
        {
            return index;
        }
    }
    return -1;
}

int WhatSonAsyncScheduler::findIntervalScheduleIndex(const QString& eventName) const
{
    for (int index = 0; index < m_intervalSchedules.size(); ++index)
    {
        if (m_intervalSchedules.at(index).eventName == eventName)
        {
            return index;
        }
    }
    return -1;
}

void WhatSonAsyncScheduler::emitScheduleTriggered(
    const QString& eventName,
    const QString& scheduleType,
    const QString& scheduleSpec,
    qint64 unixSeconds,
    const QVariantMap& payload)
{
    const QVariantMap analysis = analyzeUnixTime(unixSeconds);
    emit scheduleTriggered(
        eventName,
        scheduleType,
        scheduleSpec,
        unixSeconds,
        analysis,
        payload);
}

QString WhatSonAsyncScheduler::normalizeEventName(const QString& eventName)
{
    return eventName.trimmed();
}
