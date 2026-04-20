#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::unixTimeAnalyzer_reportsStableEpochFields()
{
    const QVariantMap epochAnalysis = WhatSon::Runtime::Scheduler::analyzeUnixSeconds(0);

    QCOMPARE(epochAnalysis.value(QStringLiteral("unixSeconds")).toLongLong(), 0);
    QCOMPARE(epochAnalysis.value(QStringLiteral("unixMilliseconds")).toLongLong(), 0);
    QCOMPARE(epochAnalysis.value(QStringLiteral("unixMinuteKey")).toLongLong(), 0);
    QCOMPARE(epochAnalysis.value(QStringLiteral("utcDate")).toString(), QStringLiteral("1970-01-01"));
    QCOMPARE(epochAnalysis.value(QStringLiteral("utcTime")).toString(), QStringLiteral("00:00:00"));
    QCOMPARE(epochAnalysis.value(QStringLiteral("utcIso")).toString(), QStringLiteral("1970-01-01T00:00:00Z"));
    QVERIFY(!epochAnalysis.value(QStringLiteral("localIso")).toString().trimmed().isEmpty());

    const int cronDayOfWeek = epochAnalysis.value(QStringLiteral("localDayOfWeekCron")).toInt();
    QVERIFY(cronDayOfWeek >= 0);
    QVERIFY(cronDayOfWeek <= 6);
    QVERIFY(WhatSon::Runtime::Scheduler::unixNowSeconds() > 0);
}
