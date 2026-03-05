#include "runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "runtime/scheduler/WhatSonCronExpression.hpp"
#include "runtime/scheduler/WhatSonUnixTimeAnalyzer.hpp"

#include <QDateTime>
#include <QSignalSpy>
#include <QTimeZone>
#include <QtTest>

class WhatSonAsyncSchedulerTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void cronExpression_mustSupportRangesStepsAndSundayAlias();
    void unixTimeAnalyzer_mustExposeStableFields();
    void asyncScheduler_mustHookAndTriggerCronAndIntervalSchedules();
};

void WhatSonAsyncSchedulerTest::cronExpression_mustSupportRangesStepsAndSundayAlias()
{
    WhatSonCronExpression weekdayExpression;
    QString parseError;
    QVERIFY2(
        weekdayExpression.parse(QStringLiteral("*/15 9-17 * * 1-5"), &parseError),
        qPrintable(parseError));

    const QTimeZone localTimeZone = QTimeZone::systemTimeZone();
    const QDateTime thursday0930(QDate(2026, 3, 5), QTime(9, 30), localTimeZone);
    QVERIFY(weekdayExpression.matches(thursday0930));

    const QDateTime thursday0931(QDate(2026, 3, 5), QTime(9, 31), localTimeZone);
    QVERIFY(!weekdayExpression.matches(thursday0931));

    const QDateTime saturday0930(QDate(2026, 3, 7), QTime(9, 30), localTimeZone);
    QVERIFY(!weekdayExpression.matches(saturday0930));

    WhatSonCronExpression sundayAliasExpression;
    QVERIFY2(
        sundayAliasExpression.parse(QStringLiteral("0 0 * * 7"), &parseError),
        qPrintable(parseError));

    const QDateTime sundayMidnight(QDate(2026, 3, 8), QTime(0, 0), localTimeZone);
    QVERIFY(sundayAliasExpression.matches(sundayMidnight));

    WhatSonCronExpression invalidExpression;
    QVERIFY(!invalidExpression.parse(QStringLiteral("* * *"), &parseError));
    QVERIFY(!parseError.trimmed().isEmpty());
}

void WhatSonAsyncSchedulerTest::unixTimeAnalyzer_mustExposeStableFields()
{
    const QVariantMap epoch = WhatSon::Runtime::Scheduler::analyzeUnixSeconds(0);

    QCOMPARE(epoch.value(QStringLiteral("unixSeconds")).toLongLong(), 0);
    QCOMPARE(epoch.value(QStringLiteral("unixMinuteKey")).toLongLong(), 0);
    QVERIFY(epoch.contains(QStringLiteral("utcIso")));
    QVERIFY(epoch.contains(QStringLiteral("localIso")));
    QVERIFY(epoch.contains(QStringLiteral("localDayOfWeekCron")));
    QVERIFY(epoch.value(QStringLiteral("utcIso")).toString().startsWith(QStringLiteral("1970-01-01")));
}

void WhatSonAsyncSchedulerTest::asyncScheduler_mustHookAndTriggerCronAndIntervalSchedules()
{
    WhatSonAsyncScheduler scheduler;

    QSignalSpy warningSpy(&scheduler, &WhatSonAsyncScheduler::schedulerWarning);
    QSignalSpy triggerSpy(&scheduler, &WhatSonAsyncScheduler::scheduleTriggered);

    const qint64 baseUnixSeconds = scheduler.unixNow();
    const QDateTime baseLocal =
        QDateTime::fromSecsSinceEpoch(baseUnixSeconds, QTimeZone::systemTimeZone());
    const QString cronExpression = QStringLiteral("%1 %2 * * *")
                                   .arg(baseLocal.time().minute())
                                   .arg(baseLocal.time().hour());

    QVERIFY(scheduler.hookCronEvent(
        QStringLiteral("cron.tick"),
        cronExpression,
        QVariantMap{{QStringLiteral("origin"), QStringLiteral("cron")}}));
    QVERIFY(scheduler.hookIntervalEvent(
        QStringLiteral("interval.tick"),
        10,
        QVariantMap{{QStringLiteral("origin"), QStringLiteral("interval")}}));

    QCOMPARE(scheduler.scheduleCount(), 2);

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(baseUnixSeconds), 1);
    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(baseUnixSeconds + 5), 0);
    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(baseUnixSeconds + 10), 1);

    QCOMPARE(triggerSpy.count(), 2);

    const QList<QVariant> firstTrigger = triggerSpy.at(0);
    QCOMPARE(firstTrigger.at(0).toString(), QStringLiteral("cron.tick"));
    QCOMPARE(firstTrigger.at(1).toString(), QStringLiteral("cron"));
    QCOMPARE(firstTrigger.at(2).toString(), cronExpression);
    QCOMPARE(firstTrigger.at(5).toMap().value(QStringLiteral("origin")).toString(), QStringLiteral("cron"));

    const QList<QVariant> secondTrigger = triggerSpy.at(1);
    QCOMPARE(secondTrigger.at(0).toString(), QStringLiteral("interval.tick"));
    QCOMPARE(secondTrigger.at(1).toString(), QStringLiteral("interval"));
    QCOMPARE(secondTrigger.at(2).toString(), QStringLiteral("10"));
    QCOMPARE(secondTrigger.at(5).toMap().value(QStringLiteral("origin")).toString(), QStringLiteral("interval"));

    QVERIFY(!scheduler.hookCronEvent(QStringLiteral("invalid"), QStringLiteral("* * *")));
    QVERIFY(warningSpy.count() >= 1);

    QVERIFY(scheduler.unhookEvent(QStringLiteral("cron.tick")));
    QCOMPARE(scheduler.scheduleCount(), 1);

    scheduler.clearSchedules();
    QCOMPARE(scheduler.scheduleCount(), 0);
}

QTEST_APPLESS_MAIN(WhatSonAsyncSchedulerTest)

#include "test_async_scheduler.moc"
