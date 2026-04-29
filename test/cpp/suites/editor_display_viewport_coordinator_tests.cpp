#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsDisplayGutterCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayStructuredFlowCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayViewportCoordinator.hpp"

void WhatSonCppRegressionTests::editorViewportCoordinator_movesMinimapAndLineMathOutOfQml()
{
    ContentsDisplayViewportCoordinator coordinator;
    coordinator.setLogicalLineCount(3);
    coordinator.setLogicalTextLength(14);
    coordinator.setMinimapResolvedTrackHeight(60.0);
    coordinator.setEditorViewportHeight(40.0);

    const QVariantList lineStartOffsets = {
        0,
        4,
        9,
    };

    QCOMPARE(coordinator.logicalLineStartOffsetAt(0, lineStartOffsets), 0);
    QCOMPARE(coordinator.logicalLineStartOffsetAt(1, lineStartOffsets), 4);
    QCOMPARE(coordinator.logicalLineStartOffsetAt(99, lineStartOffsets), 9);
    QCOMPARE(coordinator.logicalLineCharacterCountAt(0, lineStartOffsets), 3);
    QCOMPARE(coordinator.logicalLineCharacterCountAt(1, lineStartOffsets), 4);
    QCOMPARE(coordinator.logicalLineCharacterCountAt(2, lineStartOffsets), 5);
    QCOMPARE(coordinator.logicalLineNumberForOffset(0, lineStartOffsets), 1);
    QCOMPARE(coordinator.logicalLineNumberForOffset(4, lineStartOffsets), 2);
    QCOMPARE(coordinator.logicalLineNumberForOffset(8, lineStartOffsets), 2);
    QCOMPARE(coordinator.logicalLineNumberForOffset(13, lineStartOffsets), 3);

    QVERIFY(std::abs(coordinator.minimapBarWidth(0, 36.0) - 2.8) < 0.001);
    QVERIFY(std::abs(coordinator.minimapLineBarWidth(180.0, 360.0, 0, 36.0) - 17.5) < 0.001);
    QVERIFY(std::abs(coordinator.minimapLineBarWidth(480.0, 240.0, 0, 36.0) - 35.0) < 0.001);
    QVERIFY(std::abs(coordinator.minimapLineBarWidth(0.0, 360.0, 0, 36.0) - 2.8) < 0.001);
    QVERIFY(std::abs(coordinator.minimapTrackHeightForContentHeight(30.0, 120.0) - 15.0) < 0.001);
    QVERIFY(std::abs(coordinator.minimapTrackYForContentY(30.0, 120.0) - 15.0) < 0.001);
    QVERIFY(std::abs(coordinator.minimapViewportHeight(true, 120.0, 8.0) - 20.0) < 0.001);
    QVERIFY(std::abs(coordinator.minimapViewportY(true, 20.0, 120.0, 20.0) - 10.0) < 0.001);

    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString geometryControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryController.qml"));
    const QString geometrySnapshotModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometrySnapshotModel.qml"));
    const QString viewportCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayViewportCoordinator.cpp"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!geometryControllerSource.isEmpty());
    QVERIFY(!geometrySnapshotModelSource.isEmpty());
    QVERIFY(!viewportCoordinatorSource.isEmpty());
    QVERIFY(geometrySnapshotModelSource.contains(
        QStringLiteral("model.viewportCoordinator.logicalLineCharacterCountAt(")));
    QVERIFY(geometryControllerSource.contains(
        QStringLiteral("controller.viewportCoordinator.minimapViewportHeight(")));
    QVERIFY(geometryControllerSource.contains(
        QStringLiteral("controller.viewportCoordinator.minimapLineBarWidth(")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("function logicalLineCharacterCountAt(lineIndex)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("function logicalLineNumberForOffset(offset)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("function logicalLineStartOffsetAt(lineIndex)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("function minimapViewportHeight(trackHeightOverride)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("function minimapViewportY(trackHeightOverride, viewportHeightOverride)")));
    QVERIFY(viewportCoordinatorSource.contains(
        QStringLiteral("ContentsDisplayViewportCoordinator::logicalLineCharacterCountAt")));
    QVERIFY(viewportCoordinatorSource.contains(
        QStringLiteral("ContentsDisplayViewportCoordinator::minimapViewportHeight")));
    QVERIFY(viewportCoordinatorSource.contains(
        QStringLiteral("ContentsDisplayViewportCoordinator::minimapLineBarWidth")));
}

void WhatSonCppRegressionTests::editorGutterCoordinators_keepLineEntriesWhenViewportHeightIsPending()
{
    ContentsDisplayGutterCoordinator plainCoordinator;
    plainCoordinator.setLogicalLineCount(4);
    plainCoordinator.setGutterViewportHeight(0.0);

    const QVariantList plainYValues{
        0.0,
        12.0,
        24.0,
        36.0,
    };
    const QVariantList plainHeights{
        12.0,
        12.0,
        12.0,
        12.0,
    };
    const QVariantList plainRows = plainCoordinator.buildVisiblePlainGutterLineEntries(
        1,
        plainYValues,
        plainHeights);
    QCOMPARE(plainRows.size(), 4);
    QCOMPARE(plainRows.at(0).toMap().value(QStringLiteral("lineNumber")).toInt(), 1);
    QCOMPARE(plainRows.at(3).toMap().value(QStringLiteral("lineNumber")).toInt(), 4);
    QCOMPARE(plainRows.at(3).toMap().value(QStringLiteral("y")).toDouble(), 36.0);

    const QVariantList scrolledRows = plainCoordinator.buildVisiblePlainGutterLineEntries(
        3,
        QVariantList{0.0, 12.0},
        QVariantList{12.0, 12.0});
    QCOMPARE(scrolledRows.size(), 2);
    QCOMPARE(scrolledRows.at(0).toMap().value(QStringLiteral("lineNumber")).toInt(), 3);
    QCOMPARE(scrolledRows.at(0).toMap().value(QStringLiteral("y")).toDouble(), 0.0);
    QCOMPARE(scrolledRows.at(1).toMap().value(QStringLiteral("lineNumber")).toInt(), 4);
    QCOMPARE(scrolledRows.at(1).toMap().value(QStringLiteral("y")).toDouble(), 12.0);

    ContentsDisplayStructuredFlowCoordinator structuredCoordinator;
    structuredCoordinator.setStructuredHostGeometryActive(true);
    structuredCoordinator.setEditorLineHeight(12.0);
    structuredCoordinator.setGutterViewportHeight(0.0);
    structuredCoordinator.setEditorDocumentStartY(0.0);
    structuredCoordinator.setEditorContentOffsetY(0.0);

    QVariantList structuredEntries;
    for (int index = 0; index < 3; ++index)
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("contentY"), index * 12.0);
        entry.insert(QStringLiteral("contentHeight"), 12.0);
        entry.insert(QStringLiteral("gutterContentY"), index * 12.0);
        entry.insert(QStringLiteral("gutterContentHeight"), 12.0);
        structuredEntries.push_back(entry);
    }

    const QVariantList structuredRows = structuredCoordinator.buildVisibleStructuredGutterLineEntries(
        structuredEntries,
        1);
    QCOMPARE(structuredRows.size(), 3);
    QCOMPARE(structuredRows.at(0).toMap().value(QStringLiteral("lineNumber")).toInt(), 1);
    QCOMPARE(structuredRows.at(2).toMap().value(QStringLiteral("lineNumber")).toInt(), 3);
    QCOMPARE(structuredRows.at(2).toMap().value(QStringLiteral("y")).toDouble(), 24.0);
}
