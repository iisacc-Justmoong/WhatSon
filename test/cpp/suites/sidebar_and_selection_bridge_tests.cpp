#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarAndSelectionBridge_forceCppOwnershipAcrossHierarchySwitchBindings()
{
    ensureCoreApplication();

    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel resourcesViewModel(QStringLiteral("resources"));
    FakeSelectionNoteListModel libraryNoteListModel;
    FakeSelectionNoteListModel resourcesNoteListModel;
    SidebarHierarchyViewModel sidebarViewModel;
    ContentsEditorSelectionBridge selectionBridge;

    libraryNoteListModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryNoteListModel.setItemCount(5);
    libraryNoteListModel.setNoteBacked(true);

    resourcesNoteListModel.setCurrentNoteId(QStringLiteral("resource-entry"));
    resourcesNoteListModel.setItemCount(13);
    resourcesNoteListModel.setNoteBacked(false);

    libraryViewModel.setNoteListModelObject(&libraryNoteListModel);
    resourcesViewModel.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources), &resourcesViewModel},
    });
    sidebarViewModel.setViewModelProvider(&provider);

    sidebarViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources));

    QObject* activeResourcesViewModel = sidebarViewModel.activeHierarchyViewModel();
    QObject* activeResourcesNoteListModel = sidebarViewModel.activeNoteListModel();

    QCOMPARE(activeResourcesViewModel, static_cast<QObject*>(&resourcesViewModel));
    QCOMPARE(activeResourcesNoteListModel, static_cast<QObject*>(&resourcesNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);

    selectionBridge.setContentViewModel(activeResourcesViewModel);
    selectionBridge.setNoteListModel(activeResourcesNoteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.contentViewModel(), activeResourcesViewModel);
    QCOMPARE(selectionBridge.noteListModel(), activeResourcesNoteListModel);
    QVERIFY(selectionBridge.noteCountContractAvailable());
    QCOMPARE(selectionBridge.visibleNoteCount(), 13);
    QVERIFY(!selectionBridge.noteSelectionContractAvailable());

    sidebarViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library));

    QObject* activeLibraryViewModel = sidebarViewModel.activeHierarchyViewModel();
    QObject* activeLibraryNoteListModel = sidebarViewModel.activeNoteListModel();

    QCOMPARE(activeLibraryViewModel, static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(activeLibraryNoteListModel, static_cast<QObject*>(&libraryNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);

    selectionBridge.setContentViewModel(activeLibraryViewModel);
    selectionBridge.setNoteListModel(activeLibraryNoteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.contentViewModel(), activeLibraryViewModel);
    QCOMPARE(selectionBridge.noteListModel(), activeLibraryNoteListModel);
    QVERIFY(selectionBridge.noteCountContractAvailable());
    QCOMPARE(selectionBridge.visibleNoteCount(), 5);
    QVERIFY(selectionBridge.noteSelectionContractAvailable());
    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("library-note"));
    QCOMPARE(QQmlEngine::objectOwnership(activeLibraryViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(activeLibraryNoteListModel), QQmlEngine::CppOwnership);
}
