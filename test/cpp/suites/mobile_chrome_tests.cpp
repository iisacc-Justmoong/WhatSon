#include "../whatson_cpp_regression_tests.hpp"

#include "content/mobile/MobileHierarchyRouteStateStore.hpp"
#include "content/mobile/MobileHierarchySelectionCoordinator.hpp"

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

void WhatSonCppRegressionTests::mobileHierarchyRouteStateStore_tracksNormalizedSelectionRestoreState()
{
    MobileHierarchyRouteStateStore store;
    QSignalSpy routePathChangedSpy(&store, &MobileHierarchyRouteStateStore::lastObservedRoutePathChanged);
    QSignalSpy preservedSelectionChangedSpy(
        &store,
        &MobileHierarchyRouteStateStore::preservedNoteListSelectionIndexChanged);

    store.setLastObservedRoutePath(QStringLiteral("  /mobile/hierarchy  "));
    QCOMPARE(store.lastObservedRoutePath(), QStringLiteral("/mobile/hierarchy"));
    QCOMPARE(routePathChangedSpy.count(), 1);

    store.setLastObservedRoutePath(QStringLiteral("/mobile/hierarchy"));
    QCOMPARE(routePathChangedSpy.count(), 1);

    QCOMPARE(store.normalizedInteger(QVariant(QStringLiteral("12")), -1), 12);
    QCOMPARE(store.normalizedInteger(QVariant(QStringLiteral("invalid")), 7), 7);

    QCOMPARE(store.rememberSelectionIndex(QVariant(), 4), 4);
    QCOMPARE(store.preservedNoteListSelectionIndex(), 4);
    QCOMPARE(preservedSelectionChangedSpy.count(), 1);

    QCOMPARE(store.rememberSelectionIndex(QVariant(QStringLiteral("9")), 4), 9);
    QCOMPARE(store.preservedNoteListSelectionIndex(), 9);
    QCOMPARE(preservedSelectionChangedSpy.count(), 2);
    QCOMPARE(store.resolvedSelectionRestoreTarget(QVariant()), 9);
    QCOMPARE(store.resolvedSelectionRestoreTarget(QVariant(QStringLiteral("6"))), 6);
}

void WhatSonCppRegressionTests::mobileHierarchySelectionCoordinator_prefersExplicitSidebarBindingsAndFallbacks()
{
    MobileHierarchySelectionCoordinator coordinator;
    FakeHierarchyViewModel propertyViewModel(QStringLiteral("property"));
    FakeHierarchyViewModel invokedViewModel(QStringLiteral("invoked"));
    FakeMobileSidebarBindingSource sidebarBindingSource;

    sidebarBindingSource.setResolvedActiveHierarchyIndex(3);
    sidebarBindingSource.setResolvedHierarchyViewModel(
        QVariant::fromValue(static_cast<QObject*>(&propertyViewModel)));
    sidebarBindingSource.setHierarchyViewModelForIndex(3, &invokedViewModel);

    const QVariantMap bindingSnapshot = coordinator.activeHierarchyBindingSnapshotFromSidebar(
        QVariant::fromValue(static_cast<QObject*>(&sidebarBindingSource)));
    QCOMPARE(bindingSnapshot.value(QStringLiteral("index")).toInt(), 3);
    QCOMPARE(
        bindingSnapshot.value(QStringLiteral("viewModel")).value<QObject*>(),
        static_cast<QObject*>(&invokedViewModel));

    QObject activeContentViewModel;
    activeContentViewModel.setProperty("hierarchySelectedIndex", 8);
    QCOMPARE(
        coordinator.currentHierarchySelectionIndex(
            QVariant::fromValue(&activeContentViewModel),
            2),
        8);

    QObject invalidContentViewModel;
    invalidContentViewModel.setProperty("hierarchySelectedIndex", QStringLiteral("not-a-number"));
    QCOMPARE(
        coordinator.currentHierarchySelectionIndex(
            QVariant::fromValue(&invalidContentViewModel),
            5),
        5);
    QCOMPARE(coordinator.currentHierarchySelectionIndex(QVariant(), 7), 7);
}
