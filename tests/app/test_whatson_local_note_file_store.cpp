#include "file/note/WhatSonLocalNoteFileStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace
{
    QString readUtf8File(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    QList<QJsonObject> readHistoryEntries(const QString& filePath)
    {
        QList<QJsonObject> entries;
        const QStringList lines = readUtf8File(filePath).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& line : lines)
        {
            const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8());
            if (document.isObject())
            {
                entries.push_back(document.object());
            }
        }
        return entries;
    }
}

class WhatSonLocalNoteFileStoreTest final : public QObject
{
    Q_OBJECT

private slots:
    void createReadUpdateDelete_roundTripsLocalNoteFiles();
    void updateNote_headerOnlyRewrite_preservesExistingBodyText();
};

void WhatSonLocalNoteFileStoreTest::createReadUpdateDelete_roundTripsLocalNoteFiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString noteDirectoryPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wsnote"));

    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(QStringLiteral("Alpha"));
    headerStore.setCreatedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setAuthor(QStringLiteral("Tester"));
    headerStore.setLastModifiedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setModifiedBy(QStringLiteral("Tester"));
    headerStore.setFolders({QStringLiteral("Projects/One")});
    headerStore.setProject(QStringLiteral("Workspace"));
    headerStore.setBookmarked(false);
    headerStore.setBookmarkColors({});
    headerStore.setTags({QStringLiteral("alpha")});
    headerStore.setProgress(0);
    headerStore.setPreset(false);

    WhatSonLocalNoteFileStore store;
    WhatSonLocalNoteFileStore::CreateRequest createRequest;
    createRequest.noteId = QStringLiteral("Alpha");
    createRequest.noteDirectoryPath = noteDirectoryPath;
    createRequest.headerStore = headerStore;
    createRequest.bodyPlainText = QStringLiteral("alpha\nbeta");

    WhatSonLocalNoteDocument createdDocument;
    QString createError;
    QVERIFY2(store.createNote(std::move(createRequest), &createdDocument, &createError), qPrintable(createError));
    QVERIFY(QFileInfo(createdDocument.noteHeaderPath).isFile());
    QVERIFY(QFileInfo(createdDocument.noteBodyPath).isFile());
    QVERIFY(QFileInfo(createdDocument.noteHistoryPath).isFile());
    QVERIFY(QFileInfo(createdDocument.noteVersionPath).isFile());
    QVERIFY(QFileInfo(QDir(noteDirectoryPath).filePath(QStringLiteral(".meta"))).isDir());

    const QList<QJsonObject> initialHistoryEntries = readHistoryEntries(createdDocument.noteHistoryPath);
    QCOMPARE(initialHistoryEntries.size(), 1);
    QCOMPARE(initialHistoryEntries.constFirst().value(QStringLiteral("noteId")).toString(), QStringLiteral("Alpha"));
    QCOMPARE(initialHistoryEntries.constFirst().value(QStringLiteral("removedText")).toString(), QString());
    QCOMPARE(initialHistoryEntries.constFirst().value(QStringLiteral("insertedText")).toString(), QStringLiteral("alpha\nbeta"));

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteDocument readDocument;
    QString readError;
    QVERIFY2(store.readNote(readRequest, &readDocument, &readError), qPrintable(readError));
    QCOMPARE(readDocument.headerStore.noteId(), QStringLiteral("Alpha"));
    QCOMPARE(readDocument.headerStore.folders(), QStringList{QStringLiteral("Projects/One")});
    QCOMPARE(readDocument.bodyPlainText, QStringLiteral("alpha\nbeta"));
    QCOMPARE(readDocument.bodyFirstLine, QStringLiteral("alpha"));
    QCOMPARE(readDocument.noteHistoryPath, createdDocument.noteHistoryPath);
    QCOMPARE(readDocument.noteVersionPath, createdDocument.noteVersionPath);
    QVERIFY(readUtf8File(readDocument.noteHeaderPath).contains(QStringLiteral("<folder>Projects/One</folder>")));

    readDocument.bodyPlainText = QStringLiteral("gamma\ndelta");
    readDocument.headerStore.setFolders({QStringLiteral("Projects/Two")});

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = readDocument;
    updateRequest.persistHeader = true;
    updateRequest.persistBody = true;
    updateRequest.touchLastModified = true;

    WhatSonLocalNoteDocument updatedDocument;
    QString updateError;
    QVERIFY2(store.updateNote(std::move(updateRequest), &updatedDocument, &updateError), qPrintable(updateError));
    QCOMPARE(updatedDocument.bodyPlainText, QStringLiteral("gamma\ndelta"));
    QCOMPARE(updatedDocument.bodyFirstLine, QStringLiteral("gamma"));
    QCOMPARE(updatedDocument.headerStore.folders(), QStringList{QStringLiteral("Projects/Two")});
    QVERIFY(!updatedDocument.headerStore.lastModifiedAt().trimmed().isEmpty());

    const QList<QJsonObject> updatedHistoryEntries = readHistoryEntries(updatedDocument.noteHistoryPath);
    QCOMPARE(updatedHistoryEntries.size(), 2);
    QCOMPARE(updatedHistoryEntries.constLast().value(QStringLiteral("prefixLength")).toInt(), 0);
    QCOMPARE(updatedHistoryEntries.constLast().value(QStringLiteral("suffixLength")).toInt(), 2);
    QCOMPARE(updatedHistoryEntries.constLast().value(QStringLiteral("removedText")).toString(), QStringLiteral("alpha\nbe"));
    QCOMPARE(updatedHistoryEntries.constLast().value(QStringLiteral("insertedText")).toString(), QStringLiteral("gamma\ndel"));

    WhatSonLocalNoteDocument rereadDocument;
    QVERIFY2(store.readNote(readRequest, &rereadDocument, &readError), qPrintable(readError));
    QCOMPARE(rereadDocument.bodyPlainText, QStringLiteral("gamma\ndelta"));
    QCOMPARE(rereadDocument.headerStore.folders(), QStringList{QStringLiteral("Projects/Two")});

    WhatSonLocalNoteFileStore::DeleteRequest deleteRequest;
    deleteRequest.noteDirectoryPath = noteDirectoryPath;

    QString deleteError;
    QVERIFY2(store.deleteNote(std::move(deleteRequest), &deleteError), qPrintable(deleteError));
    QVERIFY(!QFileInfo(noteDirectoryPath).exists());
}

void WhatSonLocalNoteFileStoreTest::updateNote_headerOnlyRewrite_preservesExistingBodyText()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString noteDirectoryPath = QDir(tempDir.path()).filePath(QStringLiteral("Beta.wsnote"));

    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(QStringLiteral("Beta"));
    headerStore.setCreatedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setAuthor(QStringLiteral("Tester"));
    headerStore.setLastModifiedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setModifiedBy(QStringLiteral("Tester"));
    headerStore.setFolders({});
    headerStore.setProject(QString());
    headerStore.setBookmarked(false);
    headerStore.setBookmarkColors({});
    headerStore.setTags({});
    headerStore.setProgress(0);
    headerStore.setPreset(false);

    WhatSonLocalNoteFileStore store;
    WhatSonLocalNoteFileStore::CreateRequest createRequest;
    createRequest.noteId = QStringLiteral("Beta");
    createRequest.noteDirectoryPath = noteDirectoryPath;
    createRequest.headerStore = headerStore;
    createRequest.bodyPlainText = QStringLiteral("body before");

    WhatSonLocalNoteDocument createdDocument;
    QString createError;
    QVERIFY2(store.createNote(std::move(createRequest), &createdDocument, &createError), qPrintable(createError));
    QCOMPARE(readHistoryEntries(createdDocument.noteHistoryPath).size(), 1);

    createdDocument.headerStore.setFolders({QStringLiteral("Inbox/Sub")});

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = createdDocument;
    updateRequest.persistHeader = true;
    updateRequest.persistBody = false;
    updateRequest.touchLastModified = true;

    QString updateError;
    QVERIFY2(store.updateNote(std::move(updateRequest), &createdDocument, &updateError), qPrintable(updateError));

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteDocument rereadDocument;
    QString readError;
    QVERIFY2(store.readNote(readRequest, &rereadDocument, &readError), qPrintable(readError));
    QCOMPARE(rereadDocument.bodyPlainText, QStringLiteral("body before"));
    QCOMPARE(rereadDocument.headerStore.folders(), QStringList{QStringLiteral("Inbox/Sub")});
    QCOMPARE(readHistoryEntries(rereadDocument.noteHistoryPath).size(), 1);
}

QTEST_MAIN(WhatSonLocalNoteFileStoreTest)

#include "test_whatson_local_note_file_store.moc"
