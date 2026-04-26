#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsDisplayMinimapCoordinator.hpp"

void WhatSonCppRegressionTests::contentsDisplayMinimapCoordinator_splicesNormalizedSnapshotEntries()
{
    ContentsDisplayMinimapCoordinator coordinator;
    coordinator.setStructuredHostGeometryActive(true);
    coordinator.setLogicalLineCount(3);

    QVariantList currentLineGroups;
    currentLineGroups.push_back(QVariantMap { { QStringLiteral("lineNumber"), 1 } });
    currentLineGroups.push_back(QVariantMap { { QStringLiteral("lineNumber"), 2 } });
    currentLineGroups.push_back(QVariantMap { { QStringLiteral("lineNumber"), 3 } });

    QVariantList previousSnapshotEntries;
    previousSnapshotEntries.push_back(QVariantMap { { QStringLiteral("snapshotToken"), QStringLiteral("text|alpha") } });
    previousSnapshotEntries.push_back(QVariantMap { { QStringLiteral("snapshotToken"), QStringLiteral("text|beta") } });
    previousSnapshotEntries.push_back(QVariantMap { { QStringLiteral("snapshotToken"), QStringLiteral("resource|image") } });

    QVariantList identicalSnapshotEntries = previousSnapshotEntries;
    const QVariantMap identicalPlan = coordinator.buildNextMinimapSnapshotPlan(
        currentLineGroups,
        QStringLiteral("note-1"),
        QStringLiteral("note-1"),
        previousSnapshotEntries,
        identicalSnapshotEntries,
        false,
        false,
        3,
        3);
    QCOMPARE(identicalPlan.value(QStringLiteral("reuseExisting")).toBool(), true);
    QCOMPARE(identicalPlan.value(QStringLiteral("requiresFullRebuild")).toBool(), false);

    QVariantList currentSnapshotEntries;
    currentSnapshotEntries.push_back(QVariantMap { { QStringLiteral("snapshotToken"), QStringLiteral("text|alpha") } });
    currentSnapshotEntries.push_back(QVariantMap { { QStringLiteral("snapshotToken"), QStringLiteral("text|beta-updated") } });
    currentSnapshotEntries.push_back(QVariantMap { { QStringLiteral("snapshotToken"), QStringLiteral("resource|image") } });

    const QVariantMap changedPlan = coordinator.buildNextMinimapSnapshotPlan(
        currentLineGroups,
        QStringLiteral("note-1"),
        QStringLiteral("note-1"),
        previousSnapshotEntries,
        currentSnapshotEntries,
        false,
        false,
        3,
        3);
    QCOMPARE(changedPlan.value(QStringLiteral("reuseExisting")).toBool(), false);
    QCOMPARE(changedPlan.value(QStringLiteral("requiresFullRebuild")).toBool(), false);
    QCOMPARE(changedPlan.value(QStringLiteral("replacementStartLine")).toInt(), 2);
    QCOMPARE(changedPlan.value(QStringLiteral("replacementEndLine")).toInt(), 2);
    QCOMPARE(changedPlan.value(QStringLiteral("previousStartLine")).toInt(), 2);
    QCOMPARE(changedPlan.value(QStringLiteral("previousEndLine")).toInt(), 2);
}
