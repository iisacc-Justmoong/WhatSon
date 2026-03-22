#include "file/note/WhatSonLocalNoteVersionStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace
{
    QJsonObject readJsonObjectFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        return document.isObject() ? document.object() : QJsonObject{};
    }
}

class WhatSonLocalNoteVersionStoreTest final : public QObject
{
    Q_OBJECT

private slots:
    void captureSnapshot_diffSnapshots_andCheckout_roundTripWorkingTree();
    void rollbackToSnapshot_appendsRollbackSnapshotAndRestoresBody();
};

void WhatSonLocalNoteVersionStoreTest::captureSnapshot_diffSnapshots_andCheckout_roundTripWorkingTree()
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
    headerStore.setFolders({QStringLiteral("Workspace")});
    headerStore.setProject(QStringLiteral("Workspace"));
    headerStore.setBookmarked(false);
    headerStore.setBookmarkColors({});
    headerStore.setTags({});
    headerStore.setProgress(0);
    headerStore.setPreset(false);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::CreateRequest createRequest;
    createRequest.noteId = QStringLiteral("Alpha");
    createRequest.noteDirectoryPath = noteDirectoryPath;
    createRequest.headerStore = headerStore;
    createRequest.bodyPlainText = QStringLiteral("alpha");

    WhatSonLocalNoteDocument document;
    QString errorMessage;
    QVERIFY2(fileStore.createNote(std::move(createRequest), &document, &errorMessage), qPrintable(errorMessage));

    const QJsonObject initialVersionRoot = readJsonObjectFile(document.noteVersionPath);
    QCOMPARE(initialVersionRoot.value(QStringLiteral("currentSnapshotId")).toString(), QString());
    QCOMPARE(initialVersionRoot.value(QStringLiteral("headSnapshotId")).toString(), QString());
    QCOMPARE(initialVersionRoot.value(QStringLiteral("snapshots")).toArray().size(), 0);

    WhatSonLocalNoteVersionStore versionStore;
    WhatSonLocalNoteVersionStore::CaptureRequest firstCaptureRequest;
    firstCaptureRequest.document = document;
    firstCaptureRequest.label = QStringLiteral("initial");

    WhatSonNoteVersionSnapshot firstSnapshot;
    WhatSonNoteVersionState state;
    QVERIFY2(
        versionStore.captureSnapshot(std::move(firstCaptureRequest), &firstSnapshot, &state, &errorMessage),
        qPrintable(errorMessage));
    QCOMPARE(state.snapshots.size(), 1);
    QCOMPARE(state.currentSnapshotId, firstSnapshot.snapshotId);
    QCOMPARE(state.headSnapshotId, firstSnapshot.snapshotId);

    document.bodyPlainText = QStringLiteral("alpha\nbeta");
    document.headerStore.setFolders({QStringLiteral("Workspace"), QStringLiteral("Research")});
    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = document;
    updateRequest.persistHeader = true;
    updateRequest.persistBody = true;
    updateRequest.touchLastModified = true;
    QVERIFY2(fileStore.updateNote(std::move(updateRequest), &document, &errorMessage), qPrintable(errorMessage));

    WhatSonLocalNoteVersionStore::CaptureRequest secondCaptureRequest;
    secondCaptureRequest.document = document;
    secondCaptureRequest.label = QStringLiteral("expanded");

    WhatSonNoteVersionSnapshot secondSnapshot;
    QVERIFY2(
        versionStore.captureSnapshot(std::move(secondCaptureRequest), &secondSnapshot, &state, &errorMessage),
        qPrintable(errorMessage));
    QCOMPARE(state.snapshots.size(), 2);
    QCOMPARE(secondSnapshot.parentSnapshotId, firstSnapshot.snapshotId);

    WhatSonLocalNoteVersionStore::DiffRequest diffRequest;
    diffRequest.document = document;
    diffRequest.fromSnapshotId = firstSnapshot.snapshotId;
    diffRequest.toSnapshotId = secondSnapshot.snapshotId;

    WhatSonNoteVersionDiff diff;
    QVERIFY2(versionStore.diffSnapshots(std::move(diffRequest), &diff, &errorMessage), qPrintable(errorMessage));
    QCOMPARE(diff.body.prefixLength, 5);
    QCOMPARE(diff.body.suffixLength, 0);
    QCOMPARE(diff.body.removedText, QString());
    QCOMPARE(diff.body.insertedText, QStringLiteral("\nbeta"));
    QVERIFY(!diff.header.insertedText.isEmpty());

    WhatSonLocalNoteVersionStore::CheckoutRequest checkoutRequest;
    checkoutRequest.document = document;
    checkoutRequest.snapshotId = firstSnapshot.snapshotId;

    WhatSonLocalNoteDocument checkedOutDocument;
    QVERIFY2(
        versionStore.checkoutSnapshot(std::move(checkoutRequest), &checkedOutDocument, &state, &errorMessage),
        qPrintable(errorMessage));
    QCOMPARE(checkedOutDocument.bodyPlainText, QStringLiteral("alpha"));
    QCOMPARE(checkedOutDocument.headerStore.folders(), QStringList{QStringLiteral("Workspace")});
    QCOMPARE(state.currentSnapshotId, firstSnapshot.snapshotId);
    QCOMPARE(state.headSnapshotId, secondSnapshot.snapshotId);
    QCOMPARE(state.snapshots.size(), 2);
}

void WhatSonLocalNoteVersionStoreTest::rollbackToSnapshot_appendsRollbackSnapshotAndRestoresBody()
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
    headerStore.setProject(QStringLiteral("Workspace"));
    headerStore.setBookmarked(false);
    headerStore.setBookmarkColors({});
    headerStore.setTags({});
    headerStore.setProgress(0);
    headerStore.setPreset(false);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::CreateRequest createRequest;
    createRequest.noteId = QStringLiteral("Beta");
    createRequest.noteDirectoryPath = noteDirectoryPath;
    createRequest.headerStore = headerStore;
    createRequest.bodyPlainText = QStringLiteral("one");

    WhatSonLocalNoteDocument document;
    QString errorMessage;
    QVERIFY2(fileStore.createNote(std::move(createRequest), &document, &errorMessage), qPrintable(errorMessage));

    WhatSonLocalNoteVersionStore versionStore;
    WhatSonLocalNoteVersionStore::CaptureRequest firstCaptureRequest;
    firstCaptureRequest.document = document;
    firstCaptureRequest.label = QStringLiteral("first");

    WhatSonNoteVersionSnapshot firstSnapshot;
    WhatSonNoteVersionState state;
    QVERIFY2(
        versionStore.captureSnapshot(std::move(firstCaptureRequest), &firstSnapshot, &state, &errorMessage),
        qPrintable(errorMessage));

    document.bodyPlainText = QStringLiteral("two");
    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = document;
    updateRequest.persistHeader = true;
    updateRequest.persistBody = true;
    updateRequest.touchLastModified = true;
    QVERIFY2(fileStore.updateNote(std::move(updateRequest), &document, &errorMessage), qPrintable(errorMessage));

    WhatSonLocalNoteVersionStore::CaptureRequest secondCaptureRequest;
    secondCaptureRequest.document = document;
    secondCaptureRequest.label = QStringLiteral("second");

    WhatSonNoteVersionSnapshot secondSnapshot;
    QVERIFY2(
        versionStore.captureSnapshot(std::move(secondCaptureRequest), &secondSnapshot, &state, &errorMessage),
        qPrintable(errorMessage));

    WhatSonLocalNoteVersionStore::RollbackRequest rollbackRequest;
    rollbackRequest.document = document;
    rollbackRequest.snapshotId = firstSnapshot.snapshotId;
    rollbackRequest.label = QStringLiteral("rollback to first");

    WhatSonNoteVersionSnapshot rollbackSnapshot;
    WhatSonLocalNoteDocument rolledBackDocument;
    QVERIFY2(
        versionStore.rollbackToSnapshot(
            std::move(rollbackRequest),
            &rollbackSnapshot,
            &rolledBackDocument,
            &state,
            &errorMessage),
        qPrintable(errorMessage));

    QCOMPARE(rolledBackDocument.bodyPlainText, QStringLiteral("one"));
    QCOMPARE(state.snapshots.size(), 3);
    QCOMPARE(state.currentSnapshotId, rollbackSnapshot.snapshotId);
    QCOMPARE(state.headSnapshotId, rollbackSnapshot.snapshotId);
    QCOMPARE(rollbackSnapshot.parentSnapshotId, secondSnapshot.snapshotId);
    QCOMPARE(rollbackSnapshot.sourceSnapshotId, firstSnapshot.snapshotId);
    QCOMPARE(rollbackSnapshot.operation, QStringLiteral("rollback"));
}

QTEST_MAIN(WhatSonLocalNoteVersionStoreTest)

#include "test_whatson_local_note_version_store.moc"
