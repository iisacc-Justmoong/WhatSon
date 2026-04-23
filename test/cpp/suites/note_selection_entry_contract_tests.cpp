#include "test/cpp/whatson_cpp_regression_tests.hpp"

namespace
{
    LibraryNoteListItem makeLibraryNoteListItem(
        QString noteId,
        QString noteDirectoryPath,
        QString primaryText)
    {
        LibraryNoteListItem item;
        item.id = std::move(noteId);
        item.noteDirectoryPath = std::move(noteDirectoryPath);
        item.primaryText = std::move(primaryText);
        item.bodyText = QStringLiteral("Body");
        item.displayDate = QStringLiteral("2026-04-23");
        item.lastModifiedAt = QStringLiteral("2026-04-23-09-00-00");
        return item;
    }
}

void WhatSonCppRegressionTests::noteListModelContractBridge_exposesCurrentNoteEntryFromCurrentSelection()
{
    ensureCoreApplication();

    LibraryNoteListModel model;
    NoteListModelContractBridge bridge;
    QSignalSpy currentNoteEntryChangedSpy(&bridge, &NoteListModelContractBridge::currentNoteEntryChanged);
    QSignalSpy currentNoteIdChangedSpy(&bridge, &NoteListModelContractBridge::currentNoteIdChanged);

    model.setItems({
        makeLibraryNoteListItem(
            QStringLiteral("note-1"),
            QStringLiteral("/tmp/note-1.wsnote"),
            QStringLiteral("First note")),
        makeLibraryNoteListItem(
            QStringLiteral("note-2"),
            QStringLiteral("/tmp/note-2.wsnote"),
            QStringLiteral("Second note"))
    });

    bridge.setNoteListModel(&model);

    QVariantMap currentNoteEntry = bridge.currentNoteEntry();
    QCOMPARE(currentNoteEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-1"));
    QCOMPARE(
        currentNoteEntry.value(QStringLiteral("noteDirectoryPath")).toString(),
        QStringLiteral("/tmp/note-1.wsnote"));
    QCOMPARE(bridge.readCurrentNoteEntry(), currentNoteEntry);
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("note-1"));

    model.setCurrentIndex(1);

    currentNoteEntry = bridge.currentNoteEntry();
    QCOMPARE(currentNoteEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-2"));
    QCOMPARE(
        currentNoteEntry.value(QStringLiteral("noteDirectoryPath")).toString(),
        QStringLiteral("/tmp/note-2.wsnote"));
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("note-2"));
    QVERIFY(currentNoteEntryChangedSpy.count() >= 2);
    QVERIFY(currentNoteIdChangedSpy.count() >= 2);
}

void WhatSonCppRegressionTests::detailCurrentNoteContextBridge_prefersCurrentNoteEntryAndClearsNonNoteBackedSelection()
{
    ensureCoreApplication();

    DetailCurrentNoteContextBridge bridge;
    FakeContentPersistenceViewModel noteDirectorySourceViewModel;
    LibraryNoteListModel libraryNoteListModel;
    ResourcesListModel resourcesListModel;
    QSignalSpy currentNoteIdChangedSpy(&bridge, &DetailCurrentNoteContextBridge::currentNoteIdChanged);
    QSignalSpy currentNoteDirectoryPathChangedSpy(
        &bridge,
        &DetailCurrentNoteContextBridge::currentNoteDirectoryPathChanged);

    noteDirectorySourceViewModel.setNoteDirectoryPath(
        QStringLiteral("note-2"),
        QStringLiteral("/tmp/legacy-note-2.wsnote"));

    libraryNoteListModel.setItems({
        makeLibraryNoteListItem(
            QStringLiteral("note-1"),
            QStringLiteral("/tmp/library-note-1.wsnote"),
            QStringLiteral("First note")),
        makeLibraryNoteListItem(
            QStringLiteral("note-2"),
            QStringLiteral("/tmp/library-note-2.wsnote"),
            QStringLiteral("Second note"))
    });
    libraryNoteListModel.setCurrentIndex(1);

    bridge.setNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    bridge.setNoteListModel(&libraryNoteListModel);

    QCOMPARE(bridge.currentNoteId(), QStringLiteral("note-2"));
    QCOMPARE(bridge.currentNoteDirectoryPath(), QStringLiteral("/tmp/library-note-2.wsnote"));

    libraryNoteListModel.setCurrentIndex(0);

    QCOMPARE(bridge.currentNoteId(), QStringLiteral("note-1"));
    QCOMPARE(bridge.currentNoteDirectoryPath(), QStringLiteral("/tmp/library-note-1.wsnote"));

    ResourcesListItem resourceItem;
    resourceItem.id = QStringLiteral("resource-1");
    resourceItem.displayName = QStringLiteral("Cover");
    resourceItem.resourcePath = QStringLiteral("resources/cover.png");
    resourceItem.resolvedPath = QStringLiteral("/tmp/resources/cover.png");
    resourceItem.type = QStringLiteral("image");
    resourceItem.renderMode = QStringLiteral("image");
    resourcesListModel.setItems({resourceItem});
    resourcesListModel.setCurrentIndex(0);

    bridge.setNoteListModel(&resourcesListModel);

    QVERIFY(bridge.currentNoteId().isEmpty());
    QVERIFY(bridge.currentNoteDirectoryPath().isEmpty());
    QVERIFY(currentNoteIdChangedSpy.count() >= 3);
    QVERIFY(currentNoteDirectoryPathChangedSpy.count() >= 3);
}

void WhatSonCppRegressionTests::detailCurrentNoteContextBridge_clearsReadableEmptyCurrentNoteEntrySelection()
{
    ensureCoreApplication();

    DetailCurrentNoteContextBridge bridge;
    LibraryNoteListModel libraryNoteListModel;
    FakeCurrentNoteEntryOnlyListModel emptySelectionModel;
    QSignalSpy currentNoteIdChangedSpy(&bridge, &DetailCurrentNoteContextBridge::currentNoteIdChanged);
    QSignalSpy currentNoteDirectoryPathChangedSpy(
        &bridge,
        &DetailCurrentNoteContextBridge::currentNoteDirectoryPathChanged);

    libraryNoteListModel.setItems({
        makeLibraryNoteListItem(
            QStringLiteral("note-1"),
            QStringLiteral("/tmp/library-note-1.wsnote"),
            QStringLiteral("First note")),
        makeLibraryNoteListItem(
            QStringLiteral("note-2"),
            QStringLiteral("/tmp/library-note-2.wsnote"),
            QStringLiteral("Second note"))
    });
    libraryNoteListModel.setCurrentIndex(1);

    bridge.setNoteListModel(&libraryNoteListModel);

    QCOMPARE(bridge.currentNoteId(), QStringLiteral("note-2"));
    QCOMPARE(bridge.currentNoteDirectoryPath(), QStringLiteral("/tmp/library-note-2.wsnote"));

    bridge.setNoteListModel(&emptySelectionModel);

    QVERIFY(bridge.currentNoteId().isEmpty());
    QVERIFY(bridge.currentNoteDirectoryPath().isEmpty());
    QVERIFY(currentNoteIdChangedSpy.count() >= 2);
    QVERIFY(currentNoteDirectoryPathChangedSpy.count() >= 2);
}
