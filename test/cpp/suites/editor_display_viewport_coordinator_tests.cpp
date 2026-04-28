#include "test/cpp/whatson_cpp_regression_tests.hpp"

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
