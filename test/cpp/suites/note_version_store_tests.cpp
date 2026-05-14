#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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
