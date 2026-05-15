#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void WhatSonCppRegressionTests::localNoteVersionStore_splitsVersionResponsibilitiesIntoDedicatedObjects()
{
    const QString storeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/diff/WhatSonLocalNoteVersionStore.cpp"));
    const QString fileGatewayHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/diff/WhatSonNoteVersionFileGateway.hpp"));
    const QString stateCodecHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/diff/WhatSonNoteVersionStateCodec.hpp"));
    const QString diffBuilderHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/diff/WhatSonNoteVersionDiffBuilder.hpp"));
    const QString snapshotBuilderHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/diff/WhatSonNoteVersionSnapshotBuilder.hpp"));

    QVERIFY(!storeSource.isEmpty());
    QVERIFY(fileGatewayHeader.contains(QStringLiteral("class WhatSonNoteVersionFileGateway final")));
    QVERIFY(stateCodecHeader.contains(QStringLiteral("class WhatSonNoteVersionStateCodec final")));
    QVERIFY(diffBuilderHeader.contains(QStringLiteral("class WhatSonNoteVersionDiffBuilder final")));
    QVERIFY(snapshotBuilderHeader.contains(QStringLiteral("class WhatSonNoteVersionSnapshotBuilder final")));

    QVERIFY(storeSource.contains(QStringLiteral("#include \"app/models/file/diff/WhatSonNoteVersionFileGateway.hpp\"")));
    QVERIFY(storeSource.contains(QStringLiteral("#include \"app/models/file/diff/WhatSonNoteVersionStateCodec.hpp\"")));
    QVERIFY(storeSource.contains(QStringLiteral("#include \"app/models/file/diff/WhatSonNoteVersionSnapshotBuilder.hpp\"")));
    QVERIFY(storeSource.contains(QStringLiteral("WhatSonNoteVersionFileGateway fileGateway;")));
    QVERIFY(storeSource.contains(QStringLiteral("WhatSonNoteVersionStateCodec stateCodec;")));
    QVERIFY(storeSource.contains(QStringLiteral("WhatSonNoteVersionSnapshotBuilder snapshotBuilder;")));

    QVERIFY(!storeSource.contains(QStringLiteral("#include <QSaveFile>")));
    QVERIFY(!storeSource.contains(QStringLiteral("#include <QJsonDocument>")));
    QVERIFY(!storeSource.contains(QStringLiteral("#include <QCryptographicHash>")));
    QVERIFY(!storeSource.contains(QStringLiteral("#include <QRegularExpression>")));
}

void WhatSonCppRegressionTests::localNoteVersionStore_capturesCommitSnapshotWhenNoteUpdateAdvancesModifiedCount()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("versioned-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("Alpha"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read note fixture: %1").arg(readError)));

    document.bodySourceText = QStringLiteral("Beta");
    document.bodyPlainText = QStringLiteral("Beta");

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = document;
    updateRequest.touchLastModified = true;

    WhatSonLocalNoteDocument updatedDocument;
    QString updateError;
    QVERIFY2(
        fileStore.updateNote(updateRequest, &updatedDocument, &updateError),
        qPrintable(QStringLiteral("Failed to update note fixture: %1").arg(updateError)));
    QCOMPARE(updatedDocument.headerStore.modifiedCount(), 1);

    QFile versionFile(updatedDocument.noteVersionPath);
    QVERIFY2(
        versionFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(QStringLiteral("Failed to open version file: %1").arg(updatedDocument.noteVersionPath)));

    QJsonParseError parseError;
    const QJsonDocument versionDocument = QJsonDocument::fromJson(versionFile.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);

    const QJsonObject root = versionDocument.object();
    const QJsonArray snapshots = root.value(QStringLiteral("snapshots")).toArray();
    QCOMPARE(snapshots.size(), 1);

    const QJsonObject snapshot = snapshots.at(0).toObject();
    const QString snapshotId = snapshot.value(QStringLiteral("snapshotId")).toString();
    QVERIFY(!snapshotId.trimmed().isEmpty());
    QCOMPARE(root.value(QStringLiteral("currentSnapshotId")).toString(), snapshotId);
    QCOMPARE(root.value(QStringLiteral("headSnapshotId")).toString(), snapshotId);
    QCOMPARE(snapshot.value(QStringLiteral("parentSnapshotId")).toString(), QString());
    QCOMPARE(snapshot.value(QStringLiteral("label")).toString(), QStringLiteral("commit:1"));
    QCOMPARE(snapshot.value(QStringLiteral("commitModifiedCount")).toInt(), 1);
    QVERIFY(snapshot.value(QStringLiteral("headerText")).toString().contains(QStringLiteral("modifiedCount")));
    QVERIFY(snapshot.value(QStringLiteral("bodyDocumentText")).toString().contains(QStringLiteral("Beta")));
    QCOMPARE(snapshot.value(QStringLiteral("bodyPlainText")).toString(), QStringLiteral("Beta"));

    const QJsonObject headerDiff = snapshot.value(QStringLiteral("headerDiff")).toObject();
    const QJsonObject bodyDiff = snapshot.value(QStringLiteral("bodyDiff")).toObject();
    QVERIFY(headerDiff.value(QStringLiteral("unifiedPatch")).toString().contains(QStringLiteral("--- a/header.wsnhead")));
    QVERIFY(headerDiff.value(QStringLiteral("unifiedPatch")).toString().contains(QStringLiteral("+++ b/header.wsnhead")));
    QVERIFY(bodyDiff.value(QStringLiteral("unifiedPatch")).toString().contains(QStringLiteral("--- a/body.wsnbody")));
    QVERIFY(bodyDiff.value(QStringLiteral("unifiedPatch")).toString().contains(QStringLiteral("+++ b/body.wsnbody")));
    QVERIFY(bodyDiff.value(QStringLiteral("unifiedPatch")).toString().contains(QStringLiteral("Beta")));
}

void WhatSonCppRegressionTests::localNoteVersionStore_skipsModifiedCountWhenNoVersionDiffIsWritten()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("unchanged-version-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("Stable"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read note fixture: %1").arg(readError)));

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = document;
    updateRequest.persistHeader = true;
    updateRequest.persistBody = false;
    updateRequest.touchLastModified = true;
    updateRequest.incrementModifiedCount = true;

    WhatSonLocalNoteDocument updatedDocument;
    QString updateError;
    QVERIFY2(
        fileStore.updateNote(updateRequest, &updatedDocument, &updateError),
        qPrintable(QStringLiteral("Failed to update unchanged note fixture: %1").arg(updateError)));
    QCOMPARE(updatedDocument.headerStore.modifiedCount(), 0);
    QCOMPARE(updatedDocument.headerStore.lastModifiedAt(), document.headerStore.lastModifiedAt());

    QFile versionFile(updatedDocument.noteVersionPath);
    QVERIFY2(
        versionFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(QStringLiteral("Failed to open version file: %1").arg(updatedDocument.noteVersionPath)));

    QJsonParseError parseError;
    const QJsonDocument versionDocument = QJsonDocument::fromJson(versionFile.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);

    const QJsonObject root = versionDocument.object();
    QCOMPARE(root.value(QStringLiteral("snapshots")).toArray().size(), 0);
    QCOMPARE(root.value(QStringLiteral("currentSnapshotId")).toString(), QString());
    QCOMPARE(root.value(QStringLiteral("headSnapshotId")).toString(), QString());
}

void WhatSonCppRegressionTests::localNoteVersionStore_prunesSnapshotsToLatestOneHundred()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("version-limit-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("Revision 0"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read note fixture: %1").arg(readError)));

    WhatSonLocalNoteVersionStore versionStore;
    for (int index = 1; index <= 101; ++index)
    {
        const QString bodyText = QStringLiteral("Revision %1").arg(index);
        QFile bodyFile(document.noteBodyPath);
        QVERIFY2(
            bodyFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate),
            qPrintable(QStringLiteral("Failed to open body fixture for write: %1").arg(document.noteBodyPath)));
        bodyFile.write(WhatSon::NoteBodyPersistence::serializeBodyDocument(noteId, bodyText).toUtf8());
        bodyFile.close();

        WhatSonLocalNoteVersionStore::CaptureRequest captureRequest;
        captureRequest.document = document;
        captureRequest.document.bodyPlainText = bodyText;
        captureRequest.label = QStringLiteral("commit:%1").arg(index);
        captureRequest.commitModifiedCount = index;

        QString captureError;
        QVERIFY2(
            versionStore.captureSnapshot(std::move(captureRequest), nullptr, nullptr, &captureError),
            qPrintable(QStringLiteral("Failed to capture snapshot %1: %2").arg(index).arg(captureError)));
    }

    QFile versionFile(document.noteVersionPath);
    QVERIFY2(
        versionFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(QStringLiteral("Failed to open version file: %1").arg(document.noteVersionPath)));

    QJsonParseError parseError;
    const QJsonDocument versionDocument = QJsonDocument::fromJson(versionFile.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);

    const QJsonObject root = versionDocument.object();
    const QJsonArray snapshots = root.value(QStringLiteral("snapshots")).toArray();
    QCOMPARE(snapshots.size(), 100);

    const QJsonObject firstSnapshot = snapshots.first().toObject();
    QCOMPARE(firstSnapshot.value(QStringLiteral("label")).toString(), QStringLiteral("commit:2"));
    QCOMPARE(firstSnapshot.value(QStringLiteral("commitModifiedCount")).toInt(), 2);
    QCOMPARE(firstSnapshot.value(QStringLiteral("parentSnapshotId")).toString(), QString());
    QVERIFY(firstSnapshot.value(QStringLiteral("bodyDocumentText")).toString().contains(QStringLiteral("Revision 2")));

    const QJsonObject lastSnapshot = snapshots.last().toObject();
    const QString lastSnapshotId = lastSnapshot.value(QStringLiteral("snapshotId")).toString();
    QCOMPARE(lastSnapshot.value(QStringLiteral("label")).toString(), QStringLiteral("commit:101"));
    QCOMPARE(lastSnapshot.value(QStringLiteral("commitModifiedCount")).toInt(), 101);
    QVERIFY(lastSnapshot.value(QStringLiteral("bodyDocumentText")).toString().contains(QStringLiteral("Revision 101")));
    QCOMPARE(root.value(QStringLiteral("currentSnapshotId")).toString(), lastSnapshotId);
    QCOMPARE(root.value(QStringLiteral("headSnapshotId")).toString(), lastSnapshotId);
}
