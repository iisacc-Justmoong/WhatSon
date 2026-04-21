#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarHierarchyViewModel_preservesFallbackAcrossStoreAttachDetach()
{
    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel tagsViewModel(QStringLiteral("tags"));
    FakeSidebarSelectionStore selectionStore;
    SidebarHierarchyViewModel sidebarViewModel;

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsViewModel},
    });

    sidebarViewModel.setViewModelProvider(&provider);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(sidebarViewModel.activeNoteListModel(), libraryViewModel.hierarchyNoteListModel());

    QSignalSpy activeBindingsSpy(&sidebarViewModel, &IActiveHierarchyContextSource::activeBindingsChanged);
    sidebarViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(
        sidebarViewModel.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&tagsViewModel));
    QVERIFY(activeBindingsSpy.count() >= 1);

    sidebarViewModel.setSelectionStore(&selectionStore);

    QCOMPARE(
        selectionStore.selectedHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(
        sidebarViewModel.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarViewModel.activeNoteListModel(), tagsViewModel.hierarchyNoteListModel());

    selectionStore.setSelectedHierarchyIndex(999);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(sidebarViewModel.hierarchyViewModelForIndex(-1), static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(sidebarViewModel.noteListModelForIndex(-1), libraryViewModel.hierarchyNoteListModel());

    sidebarViewModel.setSelectionStore(nullptr);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&libraryViewModel));
}

void WhatSonCppRegressionTests::sidebarHierarchyViewModel_reactsToProviderMappingChanges()
{
    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel firstLibraryViewModel(QStringLiteral("library-a"));
    FakeHierarchyViewModel replacementLibraryViewModel(QStringLiteral("library-b"));
    SidebarHierarchyViewModel sidebarViewModel;

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &firstLibraryViewModel},
    });
    sidebarViewModel.setViewModelProvider(&provider);

    QSignalSpy hierarchyViewModelChangedSpy(
        &sidebarViewModel,
        &IActiveHierarchyContextSource::activeHierarchyViewModelChanged);
    QSignalSpy noteListModelChangedSpy(
        &sidebarViewModel,
        &IActiveHierarchyContextSource::activeNoteListModelChanged);
    QSignalSpy activeBindingsSpy(&sidebarViewModel, &IActiveHierarchyContextSource::activeBindingsChanged);

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &replacementLibraryViewModel},
    });

    QCOMPARE(
        sidebarViewModel.activeHierarchyViewModel(),
        static_cast<QObject*>(&replacementLibraryViewModel));
    QCOMPARE(
        sidebarViewModel.activeNoteListModel(),
        replacementLibraryViewModel.hierarchyNoteListModel());
    QVERIFY(hierarchyViewModelChangedSpy.count() >= 1);
    QVERIFY(noteListModelChangedSpy.count() >= 1);
    QVERIFY(activeBindingsSpy.count() >= 1);
}
