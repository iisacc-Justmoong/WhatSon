#include "app/runtime/scheduler/WhatSonUnixTimeAnalyzer.hpp"

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

namespace WhatSon::Runtime::Scheduler
{
    QVariantMap analyzeUnixSeconds(qint64 unixSeconds)
    {
        const QDateTime utcDateTime = QDateTime::fromSecsSinceEpoch(unixSeconds, QTimeZone::UTC);
        const QDateTime localDateTime =
            QDateTime::fromSecsSinceEpoch(unixSeconds, QTimeZone::systemTimeZone());

        int localWeekYear = 0;
        const int localWeekNumber = localDateTime.date().weekNumber(&localWeekYear);

        QVariantMap result{
            {QStringLiteral("unixSeconds"), unixSeconds},
            {QStringLiteral("unixMilliseconds"), unixSeconds * 1000},
            {QStringLiteral("unixMinuteKey"), unixSeconds / 60},
            {QStringLiteral("utcIso"), utcDateTime.toString(Qt::ISODate)},
            {QStringLiteral("localIso"), localDateTime.toString(Qt::ISODate)},
            {QStringLiteral("utcDate"), utcDateTime.date().toString(Qt::ISODate)},
            {QStringLiteral("utcTime"), utcDateTime.time().toString(Qt::ISODate)},
            {QStringLiteral("localDate"), localDateTime.date().toString(Qt::ISODate)},
            {QStringLiteral("localTime"), localDateTime.time().toString(Qt::ISODate)},
            {QStringLiteral("localOffsetFromUtcSeconds"), localDateTime.offsetFromUtc()},
            {QStringLiteral("localTimeZoneAbbreviation"), localDateTime.timeZoneAbbreviation()},
            {QStringLiteral("localDayOfWeekQt"), localDateTime.date().dayOfWeek()},
            {QStringLiteral("localDayOfWeekCron"), localDateTime.date().dayOfWeek() % 7},
            {QStringLiteral("localDayOfYear"), localDateTime.date().dayOfYear()},
            {QStringLiteral("localWeekNumber"), localWeekNumber},
            {QStringLiteral("localWeekYear"), localWeekYear},
            {QStringLiteral("localIsDaylightTime"), localDateTime.isDaylightTime()}
        };

        return result;
    }

    qint64 unixNowSeconds()
    {
        return QDateTime::currentSecsSinceEpoch();
    }
} // namespace WhatSon::Runtime::Scheduler
