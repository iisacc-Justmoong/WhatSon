#include "file/note/WhatSonHubNoteDeletionService.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

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

    QJsonObject readJsonObjectFile(const QString& filePath)
    {
        const QJsonDocument document = QJsonDocument::fromJson(readUtf8File(filePath).toUtf8());
        if (!document.isObject())
        {
            return {};
        }
        return document.object();
    }

    QStringList noteIdsFromIndexArray(const QJsonArray& notesArray)
    {
        QStringList noteIds;
        noteIds.reserve(notesArray.size());
        for (const QJsonValue& value : notesArray)
        {
            if (value.isString())
            {
                noteIds.push_back(value.toString());
                continue;
            }
            if (value.isObject())
            {
                noteIds.push_back(value.toObject().value(QStringLiteral("id")).toString());
            }
        }
        return noteIds;
    }

    bool prepareIndexedLibraryHub(QString* outHubPath)
    {
        if (outHubPath == nullptr)
        {
            return false;
        }

        static QTemporaryDir tempDir;
        if (!tempDir.isValid())
        {
            return false;
        }

        const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("NoteDeletionServiceHub.wshub"));
        QDir(hubPath).removeRecursively();

        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("NoteDeletionServiceHub.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString noteAPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
        const QString noteBPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
        const QString noteCPath = QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"));
        if (!QDir().mkpath(noteAPath) || !QDir().mkpath(noteBPath) || !QDir().mkpath(noteCPath))
        {
            return false;
        }

        const QString indexText = QStringLiteral(
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.library.index\",\n"
            "  \"notes\": [\n"
            "    {\"id\": \"note-a\"},\n"
            "    {\"id\": \"note-b\"},\n"
            "    {\"id\": \"note-c\"}\n"
            "  ]\n"
            "}\n");
        if (!writeUtf8File(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")), indexText))
        {
            return false;
        }

        if (!writeUtf8File(QDir(noteAPath).filePath(QStringLiteral("Alpha.wsnhead")), QStringLiteral("<head/>")))
        {
            return false;
        }
        if (!writeUtf8File(QDir(noteBPath).filePath(QStringLiteral("Beta.wsnhead")), QStringLiteral("<head/>")))
        {
            return false;
        }
        if (!writeUtf8File(QDir(noteCPath).filePath(QStringLiteral("Gamma.wsnhead")), QStringLiteral("<head/>")))
        {
            return false;
        }

        const QJsonObject statRoot{
            {QStringLiteral("version"), 1},
            {QStringLiteral("schema"), QStringLiteral("whatson.hub.stat")},
            {QStringLiteral("hub"), QStringLiteral("NoteDeletionServiceHub")},
            {QStringLiteral("noteCount"), 3},
            {QStringLiteral("resourceCount"), 0},
            {QStringLiteral("characterCount"), 0},
            {QStringLiteral("createdAtUtc"), QStringLiteral("2026-03-01T00:00:00Z")},
            {QStringLiteral("lastModifiedAtUtc"), QStringLiteral("2026-03-01T00:00:00Z")},
            {QStringLiteral("participants"), QJsonArray{QStringLiteral("ProfileName")}},
            {QStringLiteral("profileLastModifiedAtUtc"), QJsonObject{}}
        };
        if (!writeUtf8File(
            QDir(hubPath).filePath(QStringLiteral("NoteDeletionServiceHub.wsstat")),
            QString::fromUtf8(QJsonDocument(statRoot).toJson(QJsonDocument::Indented))))
        {
            return false;
        }

        *outHubPath = hubPath;
        return true;
    }
} // namespace

class WhatSonHubNoteDeletionServiceTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void deleteNote_removesFocusedWsnoteAndUpdatesIndexAndStat();
    void deleteNote_prunesIndexOnlyEntryWithoutWsnoteDirectory();
};

void WhatSonHubNoteDeletionServiceTest::deleteNote_removesFocusedWsnoteAndUpdatesIndexAndStat()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QString libraryPath = QDir(hubPath).filePath(
        QStringLiteral("NoteDeletionServiceHub.wscontents/Library.wslibrary"));
    const QString statPath = QDir(hubPath).filePath(QStringLiteral("NoteDeletionServiceHub.wsstat"));

    WhatSonHubStat hubStat;
    hubStat.setNoteCount(3);
    hubStat.setResourceCount(0);
    hubStat.setCharacterCount(0);
    hubStat.setCreatedAtUtc(QStringLiteral("2026-03-01T00:00:00Z"));
    hubStat.setLastModifiedAtUtc(QStringLiteral("2026-03-01T00:00:00Z"));
    hubStat.setParticipants(QStringList{QStringLiteral("ProfileName")});

    QVector<LibraryNoteRecord> notes;
    LibraryNoteRecord alpha;
    alpha.noteId = QStringLiteral("note-a");
    alpha.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
    notes.push_back(alpha);

    LibraryNoteRecord beta;
    beta.noteId = QStringLiteral("note-b");
    beta.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
    notes.push_back(beta);

    LibraryNoteRecord gamma;
    gamma.noteId = QStringLiteral("note-c");
    gamma.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"));
    notes.push_back(gamma);

    WhatSonHubNoteDeletionService service;
    WhatSonHubNoteDeletionService::Request request;
    request.wshubPath = hubPath;
    request.hubName = QStringLiteral("NoteDeletionServiceHub");
    request.hubStat = hubStat;
    request.notes = notes;
    request.noteId = QStringLiteral("note-b");

    WhatSonHubNoteDeletionService::Result result;
    QString errorMessage;
    QVERIFY2(service.deleteNote(std::move(request), &result, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(result.noteId, QStringLiteral("note-b"));
    QCOMPARE(result.remainingNotes.size(), 2);
    QCOMPARE(result.hubStat.noteCount(), 2);
    QCOMPARE(result.libraryPath, QDir::cleanPath(libraryPath));
    QCOMPARE(result.statPath, QDir::cleanPath(statPath));
    QVERIFY(!QFileInfo(QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"))).exists());

    const QJsonObject indexRoot = readJsonObjectFile(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")));
    QCOMPARE(
        noteIdsFromIndexArray(indexRoot.value(QStringLiteral("notes")).toArray()),
        QStringList({QStringLiteral("note-a"), QStringLiteral("note-c")}));

    const QJsonObject statRoot = readJsonObjectFile(statPath);
    QCOMPARE(statRoot.value(QStringLiteral("noteCount")).toInt(), 2);
    QVERIFY(!statRoot.value(QStringLiteral("lastModifiedAtUtc")).toString().isEmpty());
}

void WhatSonHubNoteDeletionServiceTest::deleteNote_prunesIndexOnlyEntryWithoutWsnoteDirectory()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QString libraryPath = QDir(hubPath).filePath(
        QStringLiteral("NoteDeletionServiceHub.wscontents/Library.wslibrary"));
    const QString statPath = QDir(hubPath).filePath(QStringLiteral("NoteDeletionServiceHub.wsstat"));
    const QString indexPath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));

    QJsonObject indexRoot = readJsonObjectFile(indexPath);
    QJsonArray notesArray = indexRoot.value(QStringLiteral("notes")).toArray();
    notesArray.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("note-orphan")}
    });
    indexRoot.insert(QStringLiteral("notes"), notesArray);
    QVERIFY(writeUtf8File(indexPath, QString::fromUtf8(QJsonDocument(indexRoot).toJson(QJsonDocument::Indented))));

    WhatSonHubStat hubStat;
    hubStat.setNoteCount(4);
    hubStat.setResourceCount(0);
    hubStat.setCharacterCount(0);
    hubStat.setCreatedAtUtc(QStringLiteral("2026-03-01T00:00:00Z"));
    hubStat.setLastModifiedAtUtc(QStringLiteral("2026-03-01T00:00:00Z"));
    hubStat.setParticipants(QStringList{QStringLiteral("ProfileName")});

    QVector<LibraryNoteRecord> notes;
    LibraryNoteRecord alpha;
    alpha.noteId = QStringLiteral("note-a");
    alpha.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
    notes.push_back(alpha);

    LibraryNoteRecord beta;
    beta.noteId = QStringLiteral("note-b");
    beta.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
    notes.push_back(beta);

    LibraryNoteRecord gamma;
    gamma.noteId = QStringLiteral("note-c");
    gamma.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"));
    notes.push_back(gamma);

    LibraryNoteRecord orphan;
    orphan.noteId = QStringLiteral("note-orphan");
    notes.push_back(orphan);

    WhatSonHubNoteDeletionService service;
    WhatSonHubNoteDeletionService::Request request;
    request.wshubPath = hubPath;
    request.hubName = QStringLiteral("NoteDeletionServiceHub");
    request.hubStat = hubStat;
    request.notes = notes;
    request.noteId = QStringLiteral("note-orphan");

    WhatSonHubNoteDeletionService::Result result;
    QString errorMessage;
    QVERIFY2(service.deleteNote(std::move(request), &result, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(result.noteId, QStringLiteral("note-orphan"));
    QCOMPARE(result.remainingNotes.size(), 3);
    QCOMPARE(result.hubStat.noteCount(), 3);
    QVERIFY(QFileInfo(QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"))).isDir());
    QVERIFY(QFileInfo(QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"))).isDir());
    QVERIFY(QFileInfo(QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"))).isDir());

    const QJsonObject healedIndexRoot = readJsonObjectFile(indexPath);
    QCOMPARE(
        noteIdsFromIndexArray(healedIndexRoot.value(QStringLiteral("notes")).toArray()),
        QStringList({QStringLiteral("note-a"), QStringLiteral("note-b"), QStringLiteral("note-c")}));

    const QJsonObject statRoot = readJsonObjectFile(statPath);
    QCOMPARE(statRoot.value(QStringLiteral("noteCount")).toInt(), 3);
    QVERIFY(!statRoot.value(QStringLiteral("lastModifiedAtUtc")).toString().isEmpty());
}

QTEST_APPLESS_MAIN(WhatSonHubNoteDeletionServiceTest)

#include "test_whatson_hub_note_deletion_service.moc"
