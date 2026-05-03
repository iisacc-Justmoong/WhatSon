#include "test/cpp/whatson_cpp_regression_tests.hpp"

namespace
{
    class ArchitecturePolicyLockResetScope final
    {
    public:
        ArchitecturePolicyLockResetScope()
        {
            WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
        }

        ~ArchitecturePolicyLockResetScope()
        {
            WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
        }
    };
}

void WhatSonCppRegressionTests::architecturePolicyLock_blocksMutableWiringAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    QVERIFY(WhatSon::Policy::verifyMutableWiringAllowed(QStringLiteral("test.mutable")));
    QVERIFY(WhatSon::Policy::verifyMutableDependencyAllowed(
        WhatSon::Policy::Layer::View,
        WhatSon::Policy::Layer::Controller,
        QStringLiteral("test.dependency")));

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    QVERIFY(!WhatSon::Policy::verifyMutableWiringAllowed(QStringLiteral("test.mutable.locked")));
    QVERIFY(!WhatSon::Policy::verifyMutableDependencyAllowed(
        WhatSon::Policy::Layer::View,
        WhatSon::Policy::Layer::Controller,
        QStringLiteral("test.dependency.locked")));
}

void WhatSonCppRegressionTests::hierarchyControllerProvider_rejectsMappingMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    HierarchyControllerProvider provider;
    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController tagsController(QStringLiteral("tags"));

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
    });
    QCOMPARE(
        provider.hierarchyController(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library)),
        static_cast<IHierarchyController*>(&libraryController));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &tagsController},
    });

    QCOMPARE(
        provider.hierarchyController(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library)),
        static_cast<IHierarchyController*>(&libraryController));
}

void WhatSonCppRegressionTests::sidebarHierarchyController_rejectsSelectionStoreMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    SidebarHierarchyController sidebarController;
    FakeSidebarSelectionStore firstStore;
    FakeSidebarSelectionStore secondStore;

    sidebarController.setSelectionStore(&firstStore);
    QCOMPARE(sidebarController.selectionStore(), static_cast<ISidebarSelectionStore*>(&firstStore));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    sidebarController.setSelectionStore(&secondStore);

    QCOMPARE(sidebarController.selectionStore(), static_cast<ISidebarSelectionStore*>(&firstStore));
}

void WhatSonCppRegressionTests::noteListModelContractBridge_rejectsWiringMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    NoteListModelContractBridge bridge;
    FakeHierarchyController firstHierarchy(QStringLiteral("first"));
    FakeHierarchyController secondHierarchy(QStringLiteral("second"));

    bridge.setHierarchyController(&firstHierarchy);
    QCOMPARE(bridge.hierarchyController(), static_cast<QObject*>(&firstHierarchy));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    bridge.setHierarchyController(&secondHierarchy);

    QCOMPARE(bridge.hierarchyController(), static_cast<QObject*>(&firstHierarchy));
}

void WhatSonCppRegressionTests::noteActiveStateTracker_rejectsHierarchyContextMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    SidebarHierarchyController firstSidebarController;
    SidebarHierarchyController secondSidebarController;
    NoteActiveStateTracker tracker;

    tracker.setHierarchyContextSource(&firstSidebarController);
    QCOMPARE(tracker.hierarchyContextSource(), static_cast<QObject*>(&firstSidebarController));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    tracker.setHierarchyContextSource(&secondSidebarController);

    QCOMPARE(tracker.hierarchyContextSource(), static_cast<QObject*>(&firstSidebarController));
}

void WhatSonCppRegressionTests::detailCurrentNoteContextBridge_rejectsWiringMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    DetailCurrentNoteContextBridge bridge;
    FakeSelectionNoteListModel firstModel;
    FakeSelectionNoteListModel secondModel;

    firstModel.setCurrentNoteId(QStringLiteral("alpha"));
    secondModel.setCurrentNoteId(QStringLiteral("beta"));

    bridge.setNoteListModel(&firstModel);
    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&firstModel));
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("alpha"));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    bridge.setNoteListModel(&secondModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&firstModel));
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("alpha"));
}

void WhatSonCppRegressionTests::onboardingRouteBootstrapController_rejectsHubControllerMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    OnboardingRouteBootstrapController controller;
    FakeOnboardingHubController firstHubController;
    FakeOnboardingHubController secondHubController;

    controller.setHubController(&firstHubController);
    WhatSon::Policy::ArchitecturePolicyLock::lock();
    controller.setHubController(&secondHubController);

    controller.configure(true, false);
    controller.handleHubLoaded();

    QCOMPARE(firstHubController.beginCount, 1);
    QCOMPARE(secondHubController.beginCount, 0);
}
