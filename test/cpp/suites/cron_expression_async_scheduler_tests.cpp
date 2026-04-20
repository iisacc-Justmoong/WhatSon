#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::cronExpression_and_asyncScheduler_coverParsingMatchingAndDeduplication()
{
    WhatSonCronExpression matcher;
    QString parseError;
    QVERIFY(matcher.parse(QStringLiteral("*/15 9-10 * * 1,3,5"), &parseError));
    QVERIFY(parseError.isEmpty());
    QCOMPARE(matcher.expression(), QStringLiteral("*/15 9-10 * * 1,3,5"));
    QVERIFY(matcher.isValid());
    QVERIFY(matcher.matches(QDateTime(QDate(2024, 4, 1), QTime(9, 30))));
    QVERIFY(!matcher.matches(QDateTime(QDate(2024, 4, 1), QTime(9, 31))));
    QVERIFY(!matcher.matches(QDateTime(QDate(2024, 4, 2), QTime(9, 30))));

    WhatSonCronExpression sundayAliasMatcher;
    QVERIFY(sundayAliasMatcher.parse(QStringLiteral("0 12 * * 7")));
    QVERIFY(sundayAliasMatcher.matches(QDateTime(QDate(2024, 3, 31), QTime(12, 0))));

    WhatSonCronExpression invalidMatcher;
    QString invalidParseError;
    QVERIFY(!invalidMatcher.parse(QStringLiteral("61 * * * *"), &invalidParseError));
    QVERIFY(!invalidParseError.trimmed().isEmpty());

    WhatSonAsyncScheduler scheduler;
    QSignalSpy schedulerWarningSpy(&scheduler, &WhatSonAsyncScheduler::schedulerWarning);
    QSignalSpy schedulesChangedSpy(&scheduler, &WhatSonAsyncScheduler::schedulesChanged);
    QSignalSpy scheduleTriggeredSpy(&scheduler, &WhatSonAsyncScheduler::scheduleTriggered);
    QSignalSpy runningChangedSpy(&scheduler, &WhatSonAsyncScheduler::runningChanged);
    QSignalSpy hookRequestCountChangedSpy(&scheduler, &WhatSonAsyncScheduler::hookRequestCountChanged);

    QVERIFY(!scheduler.hookCronEvent(QString(), QStringLiteral("* * * * *")));
    QCOMPARE(schedulerWarningSpy.count(), 1);

    QVERIFY(scheduler.hookCronEvent(
        QStringLiteral("cron"),
        QStringLiteral("* * * * *"),
        QVariantMap{{QStringLiteral("scope"), QStringLiteral("minute")}}));
    QCOMPARE(scheduler.scheduleCount(), 1);
    QVERIFY(schedulesChangedSpy.count() >= 1);

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(120), 1);
    QCOMPARE(scheduleTriggeredSpy.count(), 1);
    QCOMPARE(scheduleTriggeredSpy.at(0).at(0).toString(), QStringLiteral("cron"));
    QCOMPARE(scheduleTriggeredSpy.at(0).at(1).toString(), QStringLiteral("cron"));
    QCOMPARE(scheduleTriggeredSpy.at(0).at(2).toString(), QStringLiteral("* * * * *"));
    QCOMPARE(scheduleTriggeredSpy.at(0).at(3).toLongLong(), 120);
    QCOMPARE(
        scheduleTriggeredSpy.at(0).at(5).toMap().value(QStringLiteral("scope")).toString(),
        QStringLiteral("minute"));

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(121), 0);
    QCOMPARE(scheduleTriggeredSpy.count(), 1);

    QVERIFY(scheduler.unhookEvent(QStringLiteral("cron")));
    QCOMPARE(scheduler.scheduleCount(), 0);
    QVERIFY(!scheduler.unhookEvent(QStringLiteral("missing")));

    QVERIFY(scheduler.hookIntervalEvent(
        QStringLiteral("interval"),
        5,
        QVariantMap{{QStringLiteral("scope"), QStringLiteral("interval")}}));
    QCOMPARE(scheduler.scheduleCount(), 1);

    qint64 nextTriggerUnixSeconds = -1;
    const QVariantList schedules = scheduler.schedules();
    for (const QVariant& scheduleValue : schedules)
    {
        const QVariantMap schedule = scheduleValue.toMap();
        if (schedule.value(QStringLiteral("type")).toString() == QStringLiteral("interval"))
        {
            nextTriggerUnixSeconds = schedule.value(QStringLiteral("nextTriggerUnixSeconds")).toLongLong();
            break;
        }
    }
    QVERIFY(nextTriggerUnixSeconds > 0);

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(nextTriggerUnixSeconds - 1), 0);
    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(nextTriggerUnixSeconds), 1);
    QCOMPARE(scheduleTriggeredSpy.count(), 2);
    QCOMPARE(scheduleTriggeredSpy.at(1).at(0).toString(), QStringLiteral("interval"));
    QCOMPARE(scheduleTriggeredSpy.at(1).at(1).toString(), QStringLiteral("interval"));
    QCOMPARE(scheduleTriggeredSpy.at(1).at(2).toString(), QString::number(5));
    QCOMPARE(
        scheduleTriggeredSpy.at(1).at(5).toMap().value(QStringLiteral("scope")).toString(),
        QStringLiteral("interval"));

    scheduler.requestViewModelHook();
    QCOMPARE(scheduler.hookRequestCount(), 1);
    QCOMPARE(hookRequestCountChangedSpy.count(), 1);

    const bool started = scheduler.start();
    if (started)
    {
        QVERIFY(!scheduler.start());
        QVERIFY(scheduler.running());
        scheduler.stop();
        scheduler.stop();
        QVERIFY(!scheduler.running());
        QCOMPARE(runningChangedSpy.count(), 2);
    }
    else
    {
        QVERIFY(!scheduler.running());
        QVERIFY(!schedulerWarningSpy.isEmpty());
        QVERIFY(
            schedulerWarningSpy.constLast().at(0).toString().contains(
                QStringLiteral("could not be activated")));
        QVERIFY(!scheduler.start());
        QVERIFY(!scheduler.running());
    }

    QVERIFY(scheduler.unhookEvent(QStringLiteral("interval")));
    scheduler.clearSchedules();
    QCOMPARE(scheduler.scheduleCount(), 0);
}
