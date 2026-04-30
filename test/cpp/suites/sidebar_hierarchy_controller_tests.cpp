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
