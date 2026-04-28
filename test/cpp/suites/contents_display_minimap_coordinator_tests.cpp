#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsDisplayMinimapCoordinator.hpp"

void WhatSonCppRegressionTests::contentsDisplayMinimapCoordinator_splicesNormalizedSnapshotEntries()
{
    ContentsDisplayMinimapCoordinator coordinator;
    coordinator.setStructuredHostGeometryActive(true);
    coordinator.setEditorDocumentStartY(5.0);
    coordinator.setEditorLineHeight(12.0);
    coordinator.setLogicalLineCount(3);

    QVariantList blockEntries;
    blockEntries.push_back(QVariantMap {
        { QStringLiteral("minimapRepresentativeCharCount"), 0 },
        { QStringLiteral("minimapVisualKind"), QStringLiteral("text") },
        { QStringLiteral("plainText"), QStringLiteral("Alpha beta") },
        { QStringLiteral("sourceEnd"), 10 },
        { QStringLiteral("sourceStart"), 0 },
        { QStringLiteral("sourceText"), QStringLiteral("Alpha beta") },
        { QStringLiteral("type"), QStringLiteral("text-group") },
    });
    blockEntries.push_back(QVariantMap {
        { QStringLiteral("minimapRepresentativeCharCount"), 160 },
        { QStringLiteral("minimapVisualKind"), QStringLiteral("block") },
        { QStringLiteral("plainText"), QString() },
        { QStringLiteral("sourceEnd"), 36 },
        { QStringLiteral("sourceStart"), 10 },
        { QStringLiteral("sourceText"), QStringLiteral("<resource type=\"image\" />") },
        { QStringLiteral("type"), QStringLiteral("resource") },
    });

    const QVariantList snapshotEntries = coordinator.buildStructuredMinimapSnapshotEntries(blockEntries);
    QCOMPARE(snapshotEntries.size(), 2);
    QCOMPARE(snapshotEntries.at(0).toMap().value(QStringLiteral("charCount")).toInt(), 10);
    QCOMPARE(
        snapshotEntries.at(1).toMap().value(QStringLiteral("minimapVisualKind")).toString(),
        QStringLiteral("block"));

    const QVariantList structuredGroups = coordinator.buildStructuredMinimapLineGroupsForRange(
        snapshotEntries,
        1,
        2,
        90.0);
    QCOMPARE(structuredGroups.size(), 2);
    const QVariantMap firstStructuredGroup = structuredGroups.at(0).toMap();
    const QVariantMap secondStructuredGroup = structuredGroups.at(1).toMap();
    QCOMPARE(firstStructuredGroup.value(QStringLiteral("contentAvailableWidth")).toDouble(), 160.0);
    QCOMPARE(firstStructuredGroup.value(QStringLiteral("contentWidth")).toDouble(), 12.0);
    QCOMPARE(firstStructuredGroup.value(QStringLiteral("contentY")).toDouble(), 5.0);
    QCOMPARE(firstStructuredGroup.value(QStringLiteral("rowCount")).toInt(), 1);
    QCOMPARE(
        secondStructuredGroup.value(QStringLiteral("minimapVisualKind")).toString(),
        QStringLiteral("block"));
    QCOMPARE(secondStructuredGroup.value(QStringLiteral("contentWidth")).toDouble(), 148.0);
    QCOMPARE(secondStructuredGroup.value(QStringLiteral("contentY")).toDouble(), 35.0);
    QCOMPARE(secondStructuredGroup.value(QStringLiteral("rowCount")).toInt(), 1);

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
