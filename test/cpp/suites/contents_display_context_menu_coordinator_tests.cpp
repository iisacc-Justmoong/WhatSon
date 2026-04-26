#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsDisplayContextMenuCoordinator.hpp"

void WhatSonCppRegressionTests::displayContextMenuCoordinator_rejectsNonNumericStructuredSelectionSnapshots()
{
    ContentsDisplayContextMenuCoordinator coordinator;
    coordinator.setStructuredDocumentFlowVisible(true);
    coordinator.setStructuredContextMenuBlockIndex(2);
    coordinator.setStructuredContextMenuSelectionSnapshot({
        {QStringLiteral("selectionStart"), QStringLiteral("invalid")},
        {QStringLiteral("selectionEnd"), 5},
    });

    QVERIFY(!coordinator.structuredSelectionValid());

    QVariantMap invalidTargetState;
    invalidTargetState.insert(QStringLiteral("valid"), true);
    invalidTargetState.insert(QStringLiteral("blockIndex"), QStringLiteral("invalid"));
    invalidTargetState.insert(QStringLiteral("selectionSnapshot"), QVariantMap{
        {QStringLiteral("selectionStart"), 1},
        {QStringLiteral("selectionEnd"), 3},
        {QStringLiteral("selectedText"), QStringLiteral("abc")},
    });

    const QVariantMap invalidPlan = coordinator.primeStructuredSelectionSnapshotPlan(invalidTargetState);
    QVERIFY(!invalidPlan.value(QStringLiteral("accepted")).toBool());

    QVariantMap validTargetState;
    validTargetState.insert(QStringLiteral("valid"), true);
    validTargetState.insert(QStringLiteral("blockIndex"), 2);
    validTargetState.insert(QStringLiteral("selectionSnapshot"), QVariantMap{
        {QStringLiteral("selectionStart"), 1},
        {QStringLiteral("selectionEnd"), 4},
        {QStringLiteral("selectedText"), QStringLiteral("abc")},
    });

    const QVariantMap validPlan = coordinator.primeStructuredSelectionSnapshotPlan(validTargetState);
    QVERIFY(validPlan.value(QStringLiteral("accepted")).toBool());
    QCOMPARE(validPlan.value(QStringLiteral("blockIndex")).toInt(), 2);

    const QVariantMap normalizedSelectionSnapshot =
        validPlan.value(QStringLiteral("selectionSnapshot")).toMap();
    QCOMPARE(normalizedSelectionSnapshot.value(QStringLiteral("selectionStart")).toDouble(), 1.0);
    QCOMPARE(normalizedSelectionSnapshot.value(QStringLiteral("selectionEnd")).toDouble(), 4.0);
    QCOMPARE(normalizedSelectionSnapshot.value(QStringLiteral("selectedText")).toString(), QStringLiteral("abc"));

    const QString coordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayContextMenuCoordinator.cpp"));
    QVERIFY(!coordinatorSource.isEmpty());
    QVERIFY(!coordinatorSource.contains(
        QStringLiteral("toDouble(std::numeric_limits<double>::quiet_NaN())")));
    QVERIFY(coordinatorSource.contains(QStringLiteral("doubleOrNaN(")));
    QVERIFY(coordinatorSource.contains(QStringLiteral("toDouble(&ok)")));
}
