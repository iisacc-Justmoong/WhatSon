#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteListModelContractBridge_resolvesHierarchyBoundNoteListImmediately()
{
    ensureCoreApplication();

    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel resourcesViewModel(QStringLiteral("resources"));
    FakeSelectionNoteListModel libraryNoteListModel;
    FakeSelectionNoteListModel resourcesNoteListModel;
    NoteListModelContractBridge bridge;

    libraryNoteListModel.setCurrentIndex(2);
    libraryNoteListModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryNoteListModel.setSearchText(QStringLiteral("library-query"));

    resourcesNoteListModel.setCurrentIndex(7);
    resourcesNoteListModel.setCurrentNoteId(QStringLiteral("resource-note"));
    resourcesNoteListModel.setSearchText(QStringLiteral("resource-query"));

    libraryViewModel.setNoteListModelObject(&libraryNoteListModel);
    resourcesViewModel.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    QSignalSpy noteListModelChangedSpy(&bridge, &NoteListModelContractBridge::noteListModelChanged);

    bridge.setHierarchyViewModel(&resourcesViewModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&resourcesNoteListModel));
    QVERIFY(bridge.hasNoteListModel());
    QVERIFY(bridge.searchContractAvailable());
    QVERIFY(bridge.currentIndexContractAvailable());
    QCOMPARE(bridge.currentIndex(), 7);
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("resource-note"));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);
    QVERIFY(bridge.applySearchText(QStringLiteral("resources-filter")));
    QCOMPARE(resourcesNoteListModel.searchText(), QStringLiteral("resources-filter"));
    QVERIFY(bridge.pushCurrentIndex(3));
    QCOMPARE(resourcesNoteListModel.currentIndex(), 3);

    bridge.setHierarchyViewModel(&libraryViewModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&libraryNoteListModel));
    QVERIFY(bridge.hasNoteListModel());
    QVERIFY(bridge.searchContractAvailable());
    QVERIFY(bridge.currentIndexContractAvailable());
    QCOMPARE(bridge.currentIndex(), 2);
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("library-note"));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);
    QVERIFY(bridge.applySearchText(QStringLiteral("library-filter")));
    QCOMPARE(libraryNoteListModel.searchText(), QStringLiteral("library-filter"));
    QVERIFY(bridge.pushCurrentIndex(1));
    QCOMPARE(libraryNoteListModel.currentIndex(), 1);

    QCOMPARE(noteListModelChangedSpy.count(), 2);
}

void WhatSonCppRegressionTests::noteListModelContractBridge_prefersExplicitRowsAcrossHierarchySwitches()
{
    ensureCoreApplication();

    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel resourcesViewModel(QStringLiteral("resources"));
    LibraryNoteListModel libraryNoteListModel;
    ResourcesListModel resourcesNoteListModel;
    NoteListModelContractBridge bridge;

    LibraryNoteListItem libraryItem;
    libraryItem.id = QStringLiteral("library-note");
    libraryItem.noteDirectoryPath = QStringLiteral("/tmp/library-note.wsnote");
    libraryItem.primaryText = QStringLiteral("Library note");
    libraryItem.displayDate = QStringLiteral("2026-04-18");
    libraryItem.folders = {QStringLiteral("Marketing")};
    libraryItem.tags = {QStringLiteral("launch")};
    libraryNoteListModel.setItems({libraryItem});

    ResourcesListItem resourceItem;
    resourceItem.id = QStringLiteral("resource-entry");
    resourceItem.primaryText = QStringLiteral("Resource entry");
    resourceItem.displayDate = QStringLiteral("image");
    resourceItem.folders = {QStringLiteral("Image")};
    resourceItem.tags = {QStringLiteral(".png")};
    resourcesNoteListModel.setItems({resourceItem});

    libraryViewModel.setNoteListModelObject(&libraryNoteListModel);
    resourcesViewModel.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    bridge.setHierarchyViewModel(&resourcesViewModel);
    bridge.setNoteListModel(&resourcesNoteListModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&resourcesNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);

    QVariantList resourceRows = bridge.readAllRows();
    QCOMPARE(resourceRows.size(), 1);
    const QVariantMap resourceRow = resourceRows.constFirst().toMap();
    QCOMPARE(resourceRow.value(QStringLiteral("noteId")).toString(), QStringLiteral("resource-entry"));
    QCOMPARE(resourceRow.value(QStringLiteral("folders")).toStringList(), QStringList{QStringLiteral("Image")});
    QCOMPARE(resourceRow.value(QStringLiteral("tags")).toStringList(), QStringList{QStringLiteral(".png")});

    bridge.setHierarchyViewModel(&libraryViewModel);
    bridge.setNoteListModel(&libraryNoteListModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&libraryNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);

    QVariantList libraryRows = bridge.readAllRows();
    QCOMPARE(libraryRows.size(), 1);
    const QVariantMap libraryRow = libraryRows.constFirst().toMap();
    QCOMPARE(libraryRow.value(QStringLiteral("noteId")).toString(), QStringLiteral("library-note"));
    QCOMPARE(
        libraryRow.value(QStringLiteral("noteDirectoryPath")).toString(),
        QStringLiteral("/tmp/library-note.wsnote"));
    QCOMPARE(libraryRow.value(QStringLiteral("folders")).toStringList(), QStringList{QStringLiteral("Marketing")});
    QCOMPARE(libraryRow.value(QStringLiteral("tags")).toStringList(), QStringList{QStringLiteral("launch")});
}
