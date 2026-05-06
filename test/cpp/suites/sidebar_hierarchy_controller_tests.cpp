#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarHierarchyController_preservesFallbackAcrossStoreAttachDetach()
{
    HierarchyControllerProvider provider;
    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController tagsController(QStringLiteral("tags"));
    FakeSidebarSelectionStore selectionStore;
    SidebarHierarchyController sidebarController;

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsController},
    });

    sidebarController.setControllerProvider(&provider);

    QCOMPARE(sidebarController.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&libraryController));
    QCOMPARE(sidebarController.activeNoteListModel(), libraryController.hierarchyNoteListModel());

    QSignalSpy activeBindingsSpy(&sidebarController, &IActiveHierarchyContextSource::activeBindingsChanged);
    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(
        sidebarController.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&tagsController));
    QVERIFY(activeBindingsSpy.count() >= 1);

    sidebarController.setSelectionStore(&selectionStore);

    QCOMPARE(
        selectionStore.selectedHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(
        sidebarController.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarController.activeNoteListModel(), tagsController.hierarchyNoteListModel());

    selectionStore.setSelectedHierarchyIndex(999);

    QCOMPARE(sidebarController.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&libraryController));
    QCOMPARE(sidebarController.hierarchyControllerForIndex(-1), static_cast<QObject*>(&libraryController));
    QCOMPARE(sidebarController.noteListModelForIndex(-1), libraryController.hierarchyNoteListModel());

    sidebarController.setSelectionStore(nullptr);

    QCOMPARE(sidebarController.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&libraryController));
}

void WhatSonCppRegressionTests::sidebarHierarchyInteractionController_routesFooterActionsAndCoalescesDuplicateTriggers()
{
    SidebarHierarchyInteractionController controller;
    FakeSidebarHierarchyInteractionBridge bridge;
    controller.setHierarchyInteractionBridge(&bridge);

    QCOMPARE(
        controller.footerActionName(99, QStringLiteral(" hierarchy.footer.create ")),
        QStringLiteral("hierarchy.footer.create"));
    QCOMPARE(controller.footerActionName(0, QString()), QStringLiteral("hierarchy.footer.create"));
    QCOMPARE(controller.footerActionName(1, QString()), QStringLiteral("hierarchy.footer.delete"));
    QCOMPARE(controller.footerActionName(2, QString()), QStringLiteral("hierarchy.footer.options"));
    QCOMPARE(controller.footerActionName(3, QString()), QString());

    QSignalSpy createSpy(&controller, &SidebarHierarchyInteractionController::footerCreateRequested);
    QSignalSpy deleteSpy(&controller, &SidebarHierarchyInteractionController::footerDeleteRequested);
    QSignalSpy optionsSpy(&controller, &SidebarHierarchyInteractionController::footerOptionsRequested);

    QVERIFY(controller.requestFooterAction(QStringLiteral("hierarchy.footer.create")));
    QVERIFY(controller.requestFooterAction(QStringLiteral("hierarchy.footer.create")));
    QCOMPARE(createSpy.count(), 1);

    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    QVERIFY(controller.requestFooterAction(QStringLiteral("hierarchy.footer.delete")));
    QCOMPARE(deleteSpy.count(), 1);

    bridge.viewOptionsEnabled = false;
    QVERIFY(!controller.requestFooterAction(QStringLiteral("hierarchy.footer.options")));
    QCOMPARE(optionsSpy.count(), 0);

    bridge.viewOptionsEnabled = true;
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    QVERIFY(controller.requestFooterAction(QStringLiteral("hierarchy.footer.options")));
    QCOMPARE(optionsSpy.count(), 1);
}

void WhatSonCppRegressionTests::sidebarHierarchyInteractionController_commitsExpansionStateThroughCppPolicy()
{
    SidebarHierarchyInteractionController controller;
    FakeSidebarHierarchyInteractionBridge bridge;
    controller.setHierarchyInteractionBridge(&bridge);
    controller.setActiveHierarchyIndex(3);

    QVariantMap node{
        {QStringLiteral("itemKey"), QStringLiteral("alpha")},
        {QStringLiteral("expanded"), false},
        {QStringLiteral("showChevron"), true},
    };
    const QVariantList nodes{node};
    const QString key = controller.itemExpansionKey(node, 0);

    QCOMPARE(key, QStringLiteral("hierarchy:3:alpha"));
    controller.captureExpansionState(nodes);
    QVERIFY(controller.expansionStateContainsKey(key));
    QVERIFY(!controller.expansionStateForKey(key, true));

    const int pendingActivation = controller.beginActivationAttempt();
    node.insert(QStringLiteral("expanded"), true);
    const QVariantMap expandedResult = controller.handleExpansionSignal(node, 0, true);

    QVERIFY(expandedResult.value(QStringLiteral("committed")).toBool());
    QCOMPARE(bridge.setItemExpandedCallCount, 1);
    QCOMPARE(bridge.lastExpandedIndex, 0);
    QVERIFY(bridge.lastExpandedValue);
    QVERIFY(controller.expansionStateForKey(key, false));
    QVERIFY(!controller.activationAttemptCurrent(pendingActivation));
    QVERIFY(controller.shouldSuppressActivation());
    QTest::qWait(180);
    QVERIFY(!controller.shouldSuppressActivation());

    QVariantMap staleNode = node;
    staleNode.insert(QStringLiteral("expanded"), false);
    const QVariantList preservedModel = controller.modelWithPreservedExpansion(QVariantList{staleNode});
    QCOMPARE(preservedModel.size(), 1);
    QVERIFY(preservedModel.first().toMap().value(QStringLiteral("expanded")).toBool());

    QVERIFY(controller.armExpansionKey(key));
    const QVariantMap collapsedResult = controller.requestChevronExpansion(0, key, true, key);
    QVERIFY(collapsedResult.value(QStringLiteral("committed")).toBool());
    QCOMPARE(bridge.setItemExpandedCallCount, 2);
    QVERIFY(!bridge.lastExpandedValue);
    QVERIFY(!controller.expansionStateForKey(key, true));

    bridge.setItemExpandedResult = false;
    QVERIFY(controller.armExpansionKey(key));
    const QVariantMap failedResult = controller.requestChevronExpansion(0, key, false, key);
    QVERIFY(failedResult.value(QStringLiteral("rollbackRequired")).toBool());
    QVERIFY(!controller.expansionStateForKey(key, true));
}

void WhatSonCppRegressionTests::sidebarHierarchyController_reactsToProviderMappingChanges()
{
    HierarchyControllerProvider provider;
    FakeHierarchyController firstLibraryController(QStringLiteral("library-a"));
    FakeHierarchyController replacementLibraryController(QStringLiteral("library-b"));
    SidebarHierarchyController sidebarController;

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &firstLibraryController},
    });
    sidebarController.setControllerProvider(&provider);

    QSignalSpy hierarchyControllerChangedSpy(
        &sidebarController,
        &IActiveHierarchyContextSource::activeHierarchyControllerChanged);
    QSignalSpy noteListModelChangedSpy(
        &sidebarController,
        &IActiveHierarchyContextSource::activeNoteListModelChanged);
    QSignalSpy activeBindingsSpy(&sidebarController, &IActiveHierarchyContextSource::activeBindingsChanged);

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &replacementLibraryController},
    });

    QCOMPARE(
        sidebarController.activeHierarchyController(),
        static_cast<QObject*>(&replacementLibraryController));
    QCOMPARE(
        sidebarController.activeNoteListModel(),
        replacementLibraryController.hierarchyNoteListModel());
    QVERIFY(hierarchyControllerChangedSpy.count() >= 1);
    QVERIFY(noteListModelChangedSpy.count() >= 1);
    QVERIFY(activeBindingsSpy.count() >= 1);
}
