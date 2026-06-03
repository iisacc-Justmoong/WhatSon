#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::timestampConflictResolver_reportsStrictlyNewerTimestamp()
{
    WhatSonTimestampConflictResolver resolver;

    QVERIFY(resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-12-00-00"),
        QStringLiteral("2026-05-01-10-00-00")));
    QVERIFY(!resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-10-00-00"),
        QStringLiteral("2026-05-01-12-00-00")));
    QVERIFY(!resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-12-00-00"),
        QStringLiteral("2026-05-01-12-00-00")));
    QVERIFY(resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-12-00-00"),
        QString()));
    QVERIFY(!resolver.isTimestampNewer(
        QString(),
        QStringLiteral("2026-05-01-12-00-00")));
}
