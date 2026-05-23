#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/content/mobile/MobileHierarchyNavigationCoordinator.hpp"
#include "app/models/content/mobile/MobileHierarchyRouteStateStore.hpp"
#include "app/models/content/mobile/MobileHierarchySelectionCoordinator.hpp"

void WhatSonCppRegressionTests::mobileChrome_keepsRestoredShellWithEditorViewModeCombo()
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
    const QString internalRegistrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));

    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(!mobileHierarchyPageSource.isEmpty());
    QVERIFY(!mobileScaffoldSource.isEmpty());
    QVERIFY(!navigationBarSource.isEmpty());
    QVERIFY(!statusBarSource.isEmpty());
    QVERIFY(!internalRegistrarSource.isEmpty());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/qml/view/mobile/MobileEventSurface.qml")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/content/mobile/MobileEventSurfaceController.cpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/content/mobile/MobileEventSurfaceController.hpp")).exists());

    QVERIFY(mainQmlSource.contains(QStringLiteral("desktopMainLayoutComponent")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("mobileMainLayoutComponent")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("MobilePageView.MobileHierarchyPage {")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("mobileControlSurfaceColor")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("BodyPanelView.StatusBarLayout {")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("BodyPanelView.NavigationBarLayout {")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("BodyPanelView.BodyLayout {")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("editorOnlyWorkspaceComponent")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("PanelView.ContentViewLayout {")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("MobileView.MobileEventSurface {")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("eventSurfaceEnabled:")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("onTouchClassified")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("onScrollClassified")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("onGestureClassified")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("displayColor: mobileHierarchyPage.canvasColor")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("gutterVisible: false")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("minimapVisible: false")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileView.MobilePageScaffold {")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchyRouteStateStore")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchyCanonicalRoutePlanner")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchyPopRepairPolicy")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchyRouteSelectionSyncPolicy")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchySelectionCoordinator")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchyNavigationCoordinator")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("MobileHierarchyBackSwipeCoordinator")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("canonicalRoutePlanner.canonicalRoutePlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("selectionCoordinator.activeHierarchyBindingSnapshotFromSidebar(")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("mobileHierarchyPage.noteActiveState.activeNoteListModel")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("property var editorViewModeController: null")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("noteActiveState: mobileHierarchyPage.noteActiveState")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("property var inAppClipboard: null")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("inAppClipboard: mobileHierarchyPage.inAppClipboard")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("property var clipboardEditorPaste: null")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("clipboardEditorPaste: mobileHierarchyPage.clipboardEditorPaste")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("property var editorFontFamilyProvider: null")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("editorFontFamilyProvider: mobileHierarchyPage.editorFontFamilyProvider")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("sidebarHierarchyController: mobileHierarchyPage.sidebarHierarchyController")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("activeNoteListModelResolver")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("NoteListModelContractBridge {")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("popRepairPolicy.repairVerificationPlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("routeSelectionSyncPolicy.routeSelectionSyncPlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("navigationCoordinator.openEditorPlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("navigationCoordinator.dismissPagePlan(")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("function dismissCurrentPage()")));
    QVERIFY(!mobileHierarchyPageSource.contains(QStringLiteral("mobileScaffold.activePageRouter.back();")));
    QVERIFY(mobileHierarchyPageSource.contains(QStringLiteral("backSwipeCoordinator.beginGesturePlan(")));
    QVERIFY(mobileScaffoldSource.contains(
        QStringLiteral("property color controlSurfaceColor: LV.Theme.panelBackground10")));
    QVERIFY(mobileScaffoldSource.contains(QStringLiteral("property bool compactEditorViewVisible: false")));
    QVERIFY(mobileScaffoldSource.contains(QStringLiteral("property var editorViewModeController: null")));
    QVERIFY(mobileScaffoldSource.contains(
        QStringLiteral("compactSurfaceColor: mobilePageScaffold.controlSurfaceColor")));
    QVERIFY(mobileScaffoldSource.contains(
        QStringLiteral("compactFieldColor: mobilePageScaffold.controlSurfaceColor")));
    QVERIFY(!internalRegistrarSource.contains(QStringLiteral("MobileEventSurfaceController")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("MobileHierarchyRouteStateStore")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("MobileHierarchyNavigationCoordinator")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("MobileHierarchyBackSwipeCoordinator")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("HierarchyDragDropBridge")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("SidebarHierarchyInteractionController")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("NoteListModelContractBridge")));
    QVERIFY(internalRegistrarSource.contains(QStringLiteral("WhatSonIosHubPickerBridge")));
    QVERIFY(navigationBarSource.contains(QStringLiteral("NavigationEditorViewBar")));
    QVERIFY(navigationBarSource.contains(QStringLiteral("editorViewModeController: navigationBar.editorViewModeController")));
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
    FakeHierarchyController propertyController(QStringLiteral("property"));
    FakeHierarchyController invokedController(QStringLiteral("invoked"));
    FakeMobileSidebarBindingSource sidebarBindingSource;

    sidebarBindingSource.setResolvedActiveHierarchyIndex(3);
    sidebarBindingSource.setResolvedHierarchyController(
        QVariant::fromValue(static_cast<QObject*>(&propertyController)));
    sidebarBindingSource.setHierarchyControllerForIndex(3, &invokedController);

    const QVariantMap bindingSnapshot = coordinator.activeHierarchyBindingSnapshotFromSidebar(
        QVariant::fromValue(static_cast<QObject*>(&sidebarBindingSource)));
    QCOMPARE(bindingSnapshot.value(QStringLiteral("index")).toInt(), 3);
    QCOMPARE(
        bindingSnapshot.value(QStringLiteral("controller")).value<QObject*>(),
        static_cast<QObject*>(&invokedController));

    QObject activeContentController;
    activeContentController.setProperty("hierarchySelectedIndex", 8);
    QCOMPARE(
        coordinator.currentHierarchySelectionIndex(
            QVariant::fromValue(&activeContentController),
            2),
        8);

    QObject invalidContentController;
    invalidContentController.setProperty("hierarchySelectedIndex", QStringLiteral("not-a-number"));
    QCOMPARE(
        coordinator.currentHierarchySelectionIndex(
            QVariant::fromValue(&invalidContentController),
            5),
        5);
    QCOMPARE(coordinator.currentHierarchySelectionIndex(QVariant(), 7), 7);
}

void WhatSonCppRegressionTests::mobileHierarchyNavigationCoordinator_routesBackAsDismissTargets()
{
    MobileHierarchyNavigationCoordinator coordinator;
    coordinator.setHierarchyRoutePath(QStringLiteral("/mobile/hierarchy"));
    coordinator.setNoteListRoutePath(QStringLiteral("/mobile/note-list"));
    coordinator.setEditorRoutePath(QStringLiteral("/mobile/editor"));
    coordinator.setDetailRoutePath(QStringLiteral("/mobile/detail"));

    const QVariantMap detailPlan = coordinator.dismissPagePlan(
        true,
        true,
        QStringLiteral("/mobile/detail"));
    QVERIFY(detailPlan.value(QStringLiteral("allowed")).toBool());
    QVERIFY(detailPlan.value(QStringLiteral("dismissToEditor")).toBool());
    QCOMPARE(
        detailPlan.value(QStringLiteral("targetPath")).toString(),
        QStringLiteral("/mobile/editor"));

    const QVariantMap editorPlan = coordinator.dismissPagePlan(
        true,
        true,
        QStringLiteral("/mobile/editor"));
    QVERIFY(editorPlan.value(QStringLiteral("allowed")).toBool());
    QVERIFY(editorPlan.value(QStringLiteral("dismissToNoteList")).toBool());
    QCOMPARE(
        editorPlan.value(QStringLiteral("targetPath")).toString(),
        QStringLiteral("/mobile/note-list"));

    const QVariantMap noteListPlan = coordinator.dismissPagePlan(
        true,
        true,
        QStringLiteral("/mobile/note-list"));
    QVERIFY(noteListPlan.value(QStringLiteral("allowed")).toBool());
    QVERIFY(noteListPlan.value(QStringLiteral("dismissToHierarchy")).toBool());
    QCOMPARE(
        noteListPlan.value(QStringLiteral("targetPath")).toString(),
        QStringLiteral("/mobile/hierarchy"));

    const QVariantMap blockedEditorPlan = coordinator.dismissPagePlan(
        true,
        false,
        QStringLiteral("/mobile/editor"));
    QVERIFY(!blockedEditorPlan.value(QStringLiteral("allowed")).toBool());
    QVERIFY(!blockedEditorPlan.value(QStringLiteral("dismissToNoteList")).toBool());
}
