#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::mobileChrome_usesSharedFigmaControlSurfaceColor()
{
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));
    const QString mobileHierarchyPageSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/mobile/pages/MobileHierarchyPage.qml"));
    const QString mobileScaffoldSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/mobile/MobilePageScaffold.qml"));
    const QString navigationBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/NavigationBarLayout.qml"));
    const QString statusBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/StatusBarLayout.qml"));

    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(!mobileHierarchyPageSource.isEmpty());
    QVERIFY(!mobileScaffoldSource.isEmpty());
    QVERIFY(!navigationBarSource.isEmpty());
    QVERIFY(!statusBarSource.isEmpty());

    QVERIFY(mainQmlSource.contains(
        QStringLiteral("readonly property color mobileControlSurfaceColor: LV.Theme.panelBackground10")));
    QVERIFY(mainQmlSource.contains(
        QStringLiteral("controlSurfaceColor: applicationWindow.mobileControlSurfaceColor")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("property color controlSurfaceColor: LV.Theme.panelBackground10")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchyRouteStateStore")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchyCanonicalRoutePlanner")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchyPopRepairPolicy")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchyRouteSelectionSyncPolicy")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchySelectionCoordinator")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchyNavigationCoordinator")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("MobileHierarchyBackSwipeCoordinator")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("canonicalRoutePlanner.canonicalRoutePlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("selectionCoordinator.activeHierarchyBindingSnapshotFromSidebar(")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("popRepairPolicy.repairVerificationPlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("routeSelectionSyncPolicy.routeSelectionSyncPlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("navigationCoordinator.openEditorPlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(
        QStringLiteral("backSwipeCoordinator.beginGesturePlan(")));
    QVERIFY(mobileScaffoldSource.contains(
        QStringLiteral("property color controlSurfaceColor: LV.Theme.panelBackground10")));
    QVERIFY(mobileScaffoldSource.contains(
        QStringLiteral("compactSurfaceColor: mobilePageScaffold.controlSurfaceColor")));
    QVERIFY(mobileScaffoldSource.contains(
        QStringLiteral("compactFieldColor: mobilePageScaffold.controlSurfaceColor")));
    QVERIFY(navigationBarSource.contains(
        QStringLiteral("property color compactSurfaceColor: LV.Theme.panelBackground10")));
    QVERIFY(statusBarSource.contains(
        QStringLiteral("property color compactFieldColor: LV.Theme.panelBackground10")));
}
