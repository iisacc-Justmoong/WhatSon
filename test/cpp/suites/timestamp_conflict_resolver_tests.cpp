#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::timestampConflictResolver_prefersNewestBodyAfterBaseDivergence()
{
    WhatSonTimestampConflictResolver resolver;

    WhatSonTimestampConflictResolver::MergeRequest filesystemNewer;
    filesystemNewer.baseLastModifiedAt = QStringLiteral("2026-05-01-10-00-00");
    filesystemNewer.filesystemLastModifiedAt = QStringLiteral("2026-05-01-12-00-00");
    filesystemNewer.incomingLastModifiedAt = QStringLiteral("2026-05-01-11-00-00");
    filesystemNewer.filesystemBodySourceText = QStringLiteral("filesystem body");
    filesystemNewer.incomingBodySourceText = QStringLiteral("incoming body");

    const WhatSonTimestampConflictResolver::MergeResult filesystemResult =
        resolver.mergeBodyByTimestamp(filesystemNewer);
    QVERIFY(filesystemResult.conflictDetected);
    QCOMPARE(
        filesystemResult.winner,
        QStringLiteral("filesystem"));
    QCOMPARE(
        filesystemResult.mergedBodySourceText,
        QStringLiteral("filesystem body"));

    WhatSonTimestampConflictResolver::MergeRequest incomingNewer = filesystemNewer;
    incomingNewer.incomingLastModifiedAt = QStringLiteral("2026-05-01-13-00-00");

    const WhatSonTimestampConflictResolver::MergeResult incomingResult =
        resolver.mergeBodyByTimestamp(incomingNewer);
    QVERIFY(incomingResult.conflictDetected);
    QCOMPARE(
        incomingResult.winner,
        QStringLiteral("incoming"));
    QCOMPARE(
        incomingResult.mergedBodySourceText,
        QStringLiteral("incoming body"));
}

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
