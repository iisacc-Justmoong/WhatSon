#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/file/hierarchy/library/LibraryNotePreviewText.hpp"

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

void WhatSonCppRegressionTests::libraryNoteListModel_hidesRawInlineTagsFromPreviewText()
{
    const QString rawSource =
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>\nSecond line");

    LibraryNoteRecord record;
    record.noteId = QStringLiteral("inline-preview-note");
    record.bodyPlainText = rawSource;
    record.bodySourceText = rawSource;
    record.normalizeBodyFields();

    QCOMPARE(record.bodyPlainText, QStringLiteral("Alpha Beta\nSecond line"));
    QCOMPARE(record.bodySourceText, rawSource);
    QCOMPARE(record.bodyFirstLine, QStringLiteral("Alpha Beta"));
    QCOMPARE(
        WhatSon::LibraryPreview::notePrimaryText(record),
        QStringLiteral("Alpha Beta\nSecond line"));
    QVERIFY(!WhatSon::LibraryPreview::notePrimaryText(record).contains(QStringLiteral("<bold>")));
    QVERIFY(!WhatSon::LibraryPreview::notePrimaryText(record).contains(QStringLiteral("<italic>")));

    LibraryNoteListItem item;
    item.id = record.noteId;
    item.noteDirectoryPath = QStringLiteral("/tmp/inline-preview-note.wsnote");
    item.primaryText = WhatSon::LibraryPreview::notePrimaryText(record);
    item.bodyText = record.bodySourceText;
    item.lastModifiedAt = QStringLiteral("2026-05-07-09-00-00");

    LibraryNoteListModel model;
    model.setItems({item});

    const QModelIndex rowIndex = model.index(0, 0);
    QCOMPARE(
        model.data(rowIndex, LibraryNoteListModel::PrimaryTextRole).toString(),
        QStringLiteral("Alpha Beta\nSecond line"));
    QCOMPARE(model.data(rowIndex, LibraryNoteListModel::BodyTextRole).toString(), rawSource);
    QVERIFY(!model.data(rowIndex, LibraryNoteListModel::PrimaryTextRole).toString().contains(QStringLiteral("<")));
}
