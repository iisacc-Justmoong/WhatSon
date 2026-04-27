#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsDisplayMinimapCoordinator.hpp"

void WhatSonCppRegressionTests::contentsDisplayMinimapCoordinator_splicesNormalizedSnapshotEntries()
{
    ContentsDisplayMinimapCoordinator coordinator;
    coordinator.setStructuredHostGeometryActive(true);
    coordinator.setEditorDocumentStartY(5.0);
    coordinator.setEditorLineHeight(12.0);
    coordinator.setLogicalLineCount(3);

    QVariantList structuredLineEntries;
    structuredLineEntries.push_back(QVariantMap {
        { QStringLiteral("charCount"), 12 },
        { QStringLiteral("contentAvailableWidth"), 200.0 },
        { QStringLiteral("contentHeight"), 24.0 },
        { QStringLiteral("contentWidth"), 120.0 },
        { QStringLiteral("contentY"), 20.0 },
        { QStringLiteral("rowCount"), 2 },
        { QStringLiteral("visualRowWidths"), QVariantList { 80.0, 120.0 } },
    });

    const QVariantList structuredGroups = coordinator.buildStructuredMinimapLineGroupsForRange(
        structuredLineEntries,
        1,
        1);
    QCOMPARE(structuredGroups.size(), 1);
    const QVariantMap structuredGroup = structuredGroups.first().toMap();
    QCOMPARE(structuredGroup.value(QStringLiteral("contentAvailableWidth")).toDouble(), 200.0);
    QCOMPARE(structuredGroup.value(QStringLiteral("contentWidth")).toDouble(), 120.0);
    QCOMPARE(structuredGroup.value(QStringLiteral("contentY")).toDouble(), 25.0);
    QCOMPARE(structuredGroup.value(QStringLiteral("rowCount")).toInt(), 2);
    const QVariantList visualRowWidths = structuredGroup.value(QStringLiteral("visualRowWidths")).toList();
    QCOMPARE(visualRowWidths.size(), 2);
    QCOMPARE(visualRowWidths.at(0).toDouble(), 80.0);
    QCOMPARE(visualRowWidths.at(1).toDouble(), 120.0);

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

void WhatSonCppRegressionTests::minimapSnapshotSupport_preservesVisualRowWidths()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/models/editor/display/ContentsMinimapSnapshotSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue flattenLineGroups = library.property(QStringLiteral("flattenLineGroups"));
    QVERIFY(flattenLineGroups.isCallable());

    QJSValue visualRowWidths = engine.newArray(2);
    visualRowWidths.setProperty(0, 80.0);
    visualRowWidths.setProperty(1, 120.0);

    QJSValue group = engine.newObject();
    group.setProperty(QStringLiteral("charCount"), 12);
    group.setProperty(QStringLiteral("contentAvailableWidth"), 200.0);
    group.setProperty(QStringLiteral("contentHeight"), 24.0);
    group.setProperty(QStringLiteral("contentWidth"), 120.0);
    group.setProperty(QStringLiteral("contentY"), 10.0);
    group.setProperty(QStringLiteral("lineNumber"), 2);
    group.setProperty(QStringLiteral("rowCount"), 2);
    group.setProperty(QStringLiteral("visualRowWidths"), visualRowWidths);

    QJSValue groups = engine.newArray(1);
    groups.setProperty(0, group);
    const QJSValue rows = flattenLineGroups.call(QJSValueList {
        groups,
        QJSValue(12),
    });

    QVERIFY2(!rows.isError(), qPrintable(rows.toString()));
    QCOMPARE(rows.property(QStringLiteral("length")).toInt(), 2);

    const QJSValue firstRow = jsArrayEntry(rows, 0);
    const QJSValue secondRow = jsArrayEntry(rows, 1);
    QVERIFY(firstRow.isObject());
    QVERIFY(secondRow.isObject());
    QCOMPARE(firstRow.property(QStringLiteral("contentAvailableWidth")).toInt(), 200);
    QCOMPARE(firstRow.property(QStringLiteral("contentWidth")).toInt(), 80);
    QCOMPARE(firstRow.property(QStringLiteral("contentY")).toInt(), 10);
    QCOMPARE(secondRow.property(QStringLiteral("contentAvailableWidth")).toInt(), 200);
    QCOMPARE(secondRow.property(QStringLiteral("contentWidth")).toInt(), 120);
    QCOMPARE(secondRow.property(QStringLiteral("contentY")).toInt(), 22);
}
