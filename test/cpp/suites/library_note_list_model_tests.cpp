#include "test/cpp/whatson_cpp_regression_tests.hpp"

namespace
{
    LibraryNoteListItem makeLibraryNoteListItem(
        QString noteId,
        QString noteDirectoryPath,
        QString primaryText,
        QString bodyText)
    {
        LibraryNoteListItem item;
        item.id = std::move(noteId);
        item.noteDirectoryPath = std::move(noteDirectoryPath);
        item.primaryText = std::move(primaryText);
        item.bodyText = std::move(bodyText);
        item.displayDate = QStringLiteral("2026-04-23");
        item.lastModifiedAt = QStringLiteral("2026-04-23-09-00-00");
        return item;
    }
}

void WhatSonCppRegressionTests::libraryNoteListModel_emitsCurrentNoteEntryChangedWhenInitialSelectionMaterializes()
{
    ensureCoreApplication();

    LibraryNoteListModel model;
    QSignalSpy currentIndexChangedSpy(&model, &LibraryNoteListModel::currentIndexChanged);
    QSignalSpy currentNoteIdChangedSpy(&model, &LibraryNoteListModel::currentNoteIdChanged);
    QSignalSpy currentNoteDirectoryPathChangedSpy(&model, &LibraryNoteListModel::currentNoteDirectoryPathChanged);
    QSignalSpy currentNoteEntryChangedSpy(&model, &LibraryNoteListModel::currentNoteEntryChanged);

    model.setItems({
        makeLibraryNoteListItem(
            QStringLiteral("note-1"),
            QStringLiteral("/tmp/note-1.wsnote"),
            QStringLiteral("First note"),
            QStringLiteral("Body 1"))
    });

    QCOMPARE(model.currentIndex(), 0);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-1"));
    QCOMPARE(model.currentNoteDirectoryPath(), QStringLiteral("/tmp/note-1.wsnote"));
    QCOMPARE(currentIndexChangedSpy.count(), 1);
    QCOMPARE(currentNoteIdChangedSpy.count(), 1);
    QCOMPARE(currentNoteDirectoryPathChangedSpy.count(), 1);
    QCOMPARE(currentNoteEntryChangedSpy.count(), 1);

    const QVariantMap currentNoteEntry = model.currentNoteEntry();
    QCOMPARE(currentNoteEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-1"));
    QCOMPARE(
        currentNoteEntry.value(QStringLiteral("noteDirectoryPath")).toString(),
        QStringLiteral("/tmp/note-1.wsnote"));
    QCOMPARE(currentNoteEntry.value(QStringLiteral("primaryText")).toString(), QStringLiteral("First note"));
    QCOMPARE(currentNoteEntry.value(QStringLiteral("bodyText")).toString(), QStringLiteral("Body 1"));
}

void WhatSonCppRegressionTests::libraryNoteListModel_emitsCurrentNoteEntryChangedWhenSelectedRowReplacesCurrentSelection()
{
    ensureCoreApplication();

    LibraryNoteListModel model;
    model.setItems({
        makeLibraryNoteListItem(
            QStringLiteral("note-1"),
            QStringLiteral("/tmp/note-1.wsnote"),
            QStringLiteral("First note"),
            QStringLiteral("Body 1"))
    });

    QSignalSpy currentNoteIdChangedSpy(&model, &LibraryNoteListModel::currentNoteIdChanged);
    QSignalSpy currentNoteDirectoryPathChangedSpy(&model, &LibraryNoteListModel::currentNoteDirectoryPathChanged);
    QSignalSpy currentNoteEntryChangedSpy(&model, &LibraryNoteListModel::currentNoteEntryChanged);

    model.setItems({
        makeLibraryNoteListItem(
            QStringLiteral("note-2"),
            QStringLiteral("/tmp/note-2.wsnote"),
            QStringLiteral("Second note"),
            QStringLiteral("Body 2"))
    });

    QCOMPARE(model.currentIndex(), 0);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-2"));
    QCOMPARE(model.currentNoteDirectoryPath(), QStringLiteral("/tmp/note-2.wsnote"));
    QCOMPARE(currentNoteIdChangedSpy.count(), 1);
    QCOMPARE(currentNoteDirectoryPathChangedSpy.count(), 1);
    QCOMPARE(currentNoteEntryChangedSpy.count(), 1);

    const QVariantMap currentNoteEntry = model.currentNoteEntry();
    QCOMPARE(currentNoteEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-2"));
    QCOMPARE(
        currentNoteEntry.value(QStringLiteral("noteDirectoryPath")).toString(),
        QStringLiteral("/tmp/note-2.wsnote"));
    QCOMPARE(currentNoteEntry.value(QStringLiteral("primaryText")).toString(), QStringLiteral("Second note"));
    QCOMPARE(currentNoteEntry.value(QStringLiteral("bodyText")).toString(), QStringLiteral("Body 2"));
}
