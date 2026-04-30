#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarAndSelectionBridge_forceCppOwnershipAcrossHierarchySwitchBindings()
{
    ensureCoreApplication();

    HierarchyControllerProvider provider;
    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController resourcesController(QStringLiteral("resources"));
    FakeSelectionNoteListModel libraryNoteListModel;
    FakeSelectionNoteListModel resourcesNoteListModel;
    SidebarHierarchyController sidebarController;
    ContentsEditorSelectionBridge selectionBridge;

    libraryNoteListModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryNoteListModel.setItemCount(5);
    libraryNoteListModel.setNoteBacked(true);

    resourcesNoteListModel.setCurrentNoteId(QStringLiteral("resource-entry"));
    resourcesNoteListModel.setItemCount(13);
    resourcesNoteListModel.setNoteBacked(false);

    libraryController.setNoteListModelObject(&libraryNoteListModel);
    resourcesController.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryController, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesController, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources), &resourcesController},
    });
    sidebarController.setControllerProvider(&provider);

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources));

    QObject* activeResourcesController = sidebarController.activeHierarchyController();
    QObject* activeResourcesNoteListModel = sidebarController.activeNoteListModel();

    QCOMPARE(activeResourcesController, static_cast<QObject*>(&resourcesController));
    QCOMPARE(activeResourcesNoteListModel, static_cast<QObject*>(&resourcesNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesController), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);

    selectionBridge.setContentController(activeResourcesController);
    selectionBridge.setNoteListModel(activeResourcesNoteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.contentController(), activeResourcesController);
    QCOMPARE(selectionBridge.noteListModel(), activeResourcesNoteListModel);
    QVERIFY(selectionBridge.noteCountContractAvailable());
    QCOMPARE(selectionBridge.visibleNoteCount(), 13);
    QVERIFY(!selectionBridge.noteSelectionContractAvailable());

    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library));

    QObject* activeLibraryController = sidebarController.activeHierarchyController();
    QObject* activeLibraryNoteListModel = sidebarController.activeNoteListModel();

    QCOMPARE(activeLibraryController, static_cast<QObject*>(&libraryController));
    QCOMPARE(activeLibraryNoteListModel, static_cast<QObject*>(&libraryNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryController), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);

    selectionBridge.setContentController(activeLibraryController);
    selectionBridge.setNoteListModel(activeLibraryNoteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.contentController(), activeLibraryController);
    QCOMPARE(selectionBridge.noteListModel(), activeLibraryNoteListModel);
    QVERIFY(selectionBridge.noteCountContractAvailable());
    QCOMPARE(selectionBridge.visibleNoteCount(), 5);
    QVERIFY(selectionBridge.noteSelectionContractAvailable());
    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("library-note"));
    QCOMPARE(QQmlEngine::objectOwnership(activeLibraryController), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(activeLibraryNoteListModel), QQmlEngine::CppOwnership);
}
