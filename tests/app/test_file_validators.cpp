#include "file/validator/WhatSonHubStructureValidator.hpp"
#include "file/validator/WhatSonLibraryIndexIntegrityValidator.hpp"
#include "file/validator/WhatSonNoteStorageValidator.hpp"

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

        return file.write(text.toUtf8()) >= 0;
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

    bool prepareValidationHub(QString* outHubPath)
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

        const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ValidatorHub.wshub"));
        QDir(hubPath).removeRecursively();

        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString noteAPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
        const QString noteBPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
        const QString noteCPath = QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"));
        if (!QDir().mkpath(noteAPath) || !QDir().mkpath(noteBPath) || !QDir().mkpath(noteCPath))
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

        const QString indexText = QStringLiteral(
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.library.index\",\n"
            "  \"notes\": [\n"
            "    {\"id\": \"note-a\"},\n"
            "    {\"id\": \"note-b\"},\n"
            "    {\"id\": \"note-orphan\"},\n"
            "    {\"id\": \"note-c\"}\n"
            "  ]\n"
            "}\n");
        if (!writeUtf8File(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")), indexText))
        {
            return false;
        }

        const QJsonObject statRoot{
            {QStringLiteral("version"), 1},
            {QStringLiteral("schema"), QStringLiteral("whatson.hub.stat")},
            {QStringLiteral("hub"), QStringLiteral("ValidatorHub")},
            {QStringLiteral("noteCount"), 4}
        };
        if (!writeUtf8File(
            QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wsstat")),
            QString::fromUtf8(QJsonDocument(statRoot).toJson(QJsonDocument::Indented))))
        {
            return false;
        }

        *outHubPath = hubPath;
        return true;
    }
}

class WhatSonFileValidatorsTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void hubStructureValidator_resolvesHubPaths();
    void noteStorageValidator_resolvesMaterializedStorage();
    void libraryIndexIntegrityValidator_prunesOrphansAndRewritesIndex();
};

void WhatSonFileValidatorsTest::hubStructureValidator_resolvesHubPaths()
{
    QString hubPath;
    QVERIFY(prepareValidationHub(&hubPath));

    WhatSonHubStructureValidator validator;
    QStringList contentsDirectories;
    QString errorMessage;
    QVERIFY2(validator.resolveContentsDirectories(hubPath, &contentsDirectories, &errorMessage),
             qPrintable(errorMessage));

    QCOMPARE(contentsDirectories.size(), 1);
    QCOMPARE(
        contentsDirectories.first(),
        QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wscontents")));

    const QStringList libraryRoots = validator.resolveLibraryRoots(hubPath);
    QCOMPARE(libraryRoots, QStringList{
                 QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wscontents/Library.wslibrary"))
             });
    QCOMPARE(
        validator.resolvePrimaryLibraryPath(hubPath, &errorMessage),
        QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wscontents/Library.wslibrary")));
    QCOMPARE(
        validator.resolveHubStatPath(hubPath),
        QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wsstat")));
}

void WhatSonFileValidatorsTest::noteStorageValidator_resolvesMaterializedStorage()
{
    QString hubPath;
    QVERIFY(prepareValidationHub(&hubPath));

    const QString libraryPath = QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wscontents/Library.wslibrary"));
    const QString alphaPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
    const QString betaHeaderPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead"));

    WhatSonNoteStorageValidator validator;

    LibraryNoteRecord alpha;
    alpha.noteId = QStringLiteral("note-a");
    alpha.noteDirectoryPath = alphaPath;
    QCOMPARE(
        validator.resolveExistingNoteHeaderPath(alpha),
        QDir(alphaPath).filePath(QStringLiteral("Alpha.wsnhead")));
    QCOMPARE(validator.resolveExistingNoteDirectoryPath(alpha), alphaPath);
    QVERIFY(validator.hasMaterializedStorage(alpha));

    LibraryNoteRecord beta;
    beta.noteId = QStringLiteral("note-b");
    beta.noteHeaderPath = betaHeaderPath;
    QCOMPARE(validator.resolveExistingNoteHeaderPath(beta), betaHeaderPath);
    QCOMPARE(
        validator.resolveExistingNoteDirectoryPath(beta),
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote")));
    QVERIFY(validator.hasMaterializedStorage(beta));

    LibraryNoteRecord orphan;
    orphan.noteId = QStringLiteral("note-orphan");
    QVERIFY(validator.resolveExistingNoteHeaderPath(orphan).isEmpty());
    QVERIFY(validator.resolveExistingNoteDirectoryPath(orphan).isEmpty());
    QVERIFY(!validator.hasMaterializedStorage(orphan));
}

void WhatSonFileValidatorsTest::libraryIndexIntegrityValidator_prunesOrphansAndRewritesIndex()
{
    QString hubPath;
    QVERIFY(prepareValidationHub(&hubPath));

    const QString libraryPath = QDir(hubPath).filePath(QStringLiteral("ValidatorHub.wscontents/Library.wslibrary"));
    const QString indexPath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));

    QVector<LibraryNoteRecord> records;
    LibraryNoteRecord alpha;
    alpha.noteId = QStringLiteral("note-a");
    alpha.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
    records.push_back(alpha);

    LibraryNoteRecord beta;
    beta.noteId = QStringLiteral("note-b");
    beta.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
    records.push_back(beta);

    LibraryNoteRecord orphan;
    orphan.noteId = QStringLiteral("note-orphan");
    records.push_back(orphan);

    LibraryNoteRecord gamma;
    gamma.noteId = QStringLiteral("note-c");
    gamma.noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"));
    records.push_back(gamma);

    WhatSonLibraryIndexIntegrityValidator validator;
    const WhatSonLibraryIndexIntegrityValidator::PruneResult pruneResult = validator.pruneOrphanRecords(records);

    QCOMPARE(pruneResult.materializedRecords.size(), 3);
    QCOMPARE(pruneResult.prunedOrphanNoteIds, QStringList{QStringLiteral("note-orphan")});

    QString errorMessage;
    QVERIFY2(
        validator.rewriteIndexesFromRecords(
            hubPath,
            QStringList{libraryPath},
            pruneResult.materializedRecords,
            &errorMessage),
        qPrintable(errorMessage));

    const QJsonDocument indexDocument = QJsonDocument::fromJson(readUtf8File(indexPath).toUtf8());
    QVERIFY(indexDocument.isObject());
    QCOMPARE(
        noteIdsFromIndexArray(indexDocument.object().value(QStringLiteral("notes")).toArray()),
        QStringList({QStringLiteral("note-a"), QStringLiteral("note-b"), QStringLiteral("note-c")}));
}

QTEST_APPLESS_MAIN(WhatSonFileValidatorsTest)

#include "test_file_validators.moc"
