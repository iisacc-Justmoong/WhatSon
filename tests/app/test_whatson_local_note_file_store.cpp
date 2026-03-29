#include "file/note/WhatSonLocalNoteFileStore.hpp"
#include "file/note/WhatSonNoteHeaderCreator.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest/QtTest>

namespace
{
    bool writeUtf8File(const QString& filePath, const QString& text)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        file.write(text.toUtf8());
        return true;
    }

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

    bool createResourcePackage(
        const QString& resourcesPath,
        const QString& resourceId,
        const QString& assetFileName,
        const QString& assetPayload = QStringLiteral("payload"))
    {
        const QString packageDirectoryPath = QDir(resourcesPath).filePath(resourceId + QStringLiteral(".wsresource"));
        if (!QDir().mkpath(packageDirectoryPath))
        {
            return false;
        }

        const QString resourcePath = QStringLiteral("%1/%2")
                                         .arg(QFileInfo(resourcesPath).fileName(), QFileInfo(packageDirectoryPath).fileName());
        WhatSon::Resources::ResourcePackageMetadata metadata = WhatSon::Resources::buildMetadataForAssetFile(
            assetFileName,
            resourceId,
            resourcePath);

        return writeUtf8File(QDir(packageDirectoryPath).filePath(assetFileName), assetPayload)
            && writeUtf8File(
                QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName()),
                WhatSon::Resources::createResourcePackageMetadataXml(metadata));
    }
}

class WhatSonLocalNoteFileStoreTest final : public QObject
{
    Q_OBJECT

private slots:
    void createReadUpdateDelete_roundTripsLocalNoteFiles();
    void readNote_resolvesWsresourceThumbnailFromBody();
    void readNote_preservesEmptyAndWhitespaceOnlyParagraphs();
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

void WhatSonLocalNoteFileStoreTest::readNote_resolvesWsresourceThumbnailFromBody()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("PreviewHub.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("PreviewHub.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Preview.wsnote"));
    const QString resourcesPath = QDir(hubPath).filePath(QStringLiteral("PreviewHub.wsresources"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));
    QVERIFY(QDir().mkpath(resourcesPath));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("preview"),
        QStringLiteral("preview.png"),
        QStringLiteral("png")));

    const QString bodyXml =
        QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE WHATSONNOTE>\n"
            "<contents id=\"Preview\">\n"
            "  <body>\n"
            "    <resource type=\"image\" format=\".png\" resourcePath=\"PreviewHub.wsresources/preview.wsresource\" />\n"
            "    <paragraph>Preview line</paragraph>\n"
            "  </body>\n"
            "</contents>\n");

    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(QStringLiteral("Preview"));
    headerStore.setCreatedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setAuthor(QStringLiteral("Tester"));
    headerStore.setLastModifiedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setModifiedBy(QStringLiteral("Tester"));
    headerStore.setProgress(0);
    headerStore.setPreset(false);

    WhatSonNoteHeaderCreator headerCreator(hubPath);
    QVERIFY(writeUtf8File(
        QDir(noteDirectoryPath).filePath(QStringLiteral("Preview.wsnhead")),
        headerCreator.createHeaderText(headerStore)));
    QVERIFY(writeUtf8File(QDir(noteDirectoryPath).filePath(QStringLiteral("Preview.wsnbody")), bodyXml));
    QVERIFY(writeUtf8File(QDir(noteDirectoryPath).filePath(QStringLiteral("Preview.wsnhistory")), QString()));
    QVERIFY(writeUtf8File(QDir(noteDirectoryPath).filePath(QStringLiteral("Preview.wsnversion")), QStringLiteral("{}")));

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteFileStore store;
    WhatSonLocalNoteDocument document;
    QString errorMessage;
    QVERIFY2(store.readNote(readRequest, &document, &errorMessage), qPrintable(errorMessage));
    QVERIFY(document.bodyHasResource);
    QCOMPARE(
        document.bodyFirstResourceThumbnailUrl,
        QUrl::fromLocalFile(
            QDir(resourcesPath).filePath(QStringLiteral("preview.wsresource/preview.png"))).toString());
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

void WhatSonLocalNoteFileStoreTest::readNote_preservesEmptyAndWhitespaceOnlyParagraphs()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString noteDirectoryPath = QDir(tempDir.path()).filePath(QStringLiteral("Gamma.wsnote"));

    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(QStringLiteral("Gamma"));
    headerStore.setCreatedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setAuthor(QStringLiteral("Tester"));
    headerStore.setLastModifiedAt(QStringLiteral("2026-03-22-12-00-00"));
    headerStore.setModifiedBy(QStringLiteral("Tester"));

    WhatSonLocalNoteFileStore store;
    WhatSonLocalNoteFileStore::CreateRequest createRequest;
    createRequest.noteId = QStringLiteral("Gamma");
    createRequest.noteDirectoryPath = noteDirectoryPath;
    createRequest.headerStore = headerStore;
    createRequest.bodyPlainText = QStringLiteral("seed");

    WhatSonLocalNoteDocument createdDocument;
    QString createError;
    QVERIFY2(store.createNote(std::move(createRequest), &createdDocument, &createError), qPrintable(createError));

    const QString bodyXml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"Gamma\">\n"
        "  <body>\n"
        "    <paragraph>alpha</paragraph>\n"
        "    <paragraph></paragraph>\n"
        "    <paragraph>   </paragraph>\n"
        "    <paragraph>beta</paragraph>\n"
        "  </body>\n"
        "</contents>\n");
    QVERIFY(writeUtf8File(createdDocument.noteBodyPath, bodyXml));

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteDocument readDocument;
    QString readError;
    QVERIFY2(store.readNote(readRequest, &readDocument, &readError), qPrintable(readError));
    QCOMPARE(readDocument.bodyPlainText, QStringLiteral("alpha\n\n   \nbeta"));
    QCOMPARE(readDocument.bodyFirstLine, QStringLiteral("alpha"));
}

QTEST_MAIN(WhatSonLocalNoteFileStoreTest)

#include "test_whatson_local_note_file_store.moc"
