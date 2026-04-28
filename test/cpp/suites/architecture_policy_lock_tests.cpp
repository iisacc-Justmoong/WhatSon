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
        WhatSon::Policy::Layer::ViewModel,
        QStringLiteral("test.dependency")));

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    QVERIFY(!WhatSon::Policy::verifyMutableWiringAllowed(QStringLiteral("test.mutable.locked")));
    QVERIFY(!WhatSon::Policy::verifyMutableDependencyAllowed(
        WhatSon::Policy::Layer::View,
        WhatSon::Policy::Layer::ViewModel,
        QStringLiteral("test.dependency.locked")));
}

void WhatSonCppRegressionTests::hierarchyViewModelProvider_rejectsMappingMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel tagsViewModel(QStringLiteral("tags"));

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
    });
    QCOMPARE(
        provider.hierarchyViewModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library)),
        static_cast<IHierarchyViewModel*>(&libraryViewModel));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &tagsViewModel},
    });

    QCOMPARE(
        provider.hierarchyViewModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library)),
        static_cast<IHierarchyViewModel*>(&libraryViewModel));
}

void WhatSonCppRegressionTests::sidebarHierarchyViewModel_rejectsSelectionStoreMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    SidebarHierarchyViewModel sidebarViewModel;
    FakeSidebarSelectionStore firstStore;
    FakeSidebarSelectionStore secondStore;

    sidebarViewModel.setSelectionStore(&firstStore);
    QCOMPARE(sidebarViewModel.selectionStore(), static_cast<ISidebarSelectionStore*>(&firstStore));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    sidebarViewModel.setSelectionStore(&secondStore);

    QCOMPARE(sidebarViewModel.selectionStore(), static_cast<ISidebarSelectionStore*>(&firstStore));
}

void WhatSonCppRegressionTests::noteListModelContractBridge_rejectsWiringMutationAfterLock()
{
    ArchitecturePolicyLockResetScope resetScope;

    NoteListModelContractBridge bridge;
    FakeHierarchyViewModel firstHierarchy(QStringLiteral("first"));
    FakeHierarchyViewModel secondHierarchy(QStringLiteral("second"));

    bridge.setHierarchyViewModel(&firstHierarchy);
    QCOMPARE(bridge.hierarchyViewModel(), static_cast<QObject*>(&firstHierarchy));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    bridge.setHierarchyViewModel(&secondHierarchy);

    QCOMPARE(bridge.hierarchyViewModel(), static_cast<QObject*>(&firstHierarchy));
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
