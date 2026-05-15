#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::timestampConflictResolver_prefersNewestBodyAfterBaseDivergence()
{
    WhatSonTimestampConflictResolver resolver;

    WhatSonTimestampConflictResolver::MergeRequest filesystemNewer;
    filesystemNewer.baseLastModifiedAt = QStringLiteral("2026-05-01-10-00-00");
    filesystemNewer.filesystemLastModifiedAt = QStringLiteral("2026-05-01-12-00-00");
    filesystemNewer.incomingLastModifiedAt = QStringLiteral("2026-05-01-11-00-00");
    filesystemNewer.filesystemBodySourceText = QStringLiteral("filesystem body");
    filesystemNewer.incomingBodySourceText = QStringLiteral("incoming body");

    const WhatSonTimestampConflictResolver::MergeResult filesystemResult =
        resolver.mergeBodyByTimestamp(filesystemNewer);
    QVERIFY(filesystemResult.conflictDetected);
    QCOMPARE(
        filesystemResult.winner,
        QStringLiteral("filesystem"));
    QCOMPARE(
        filesystemResult.mergedBodySourceText,
        QStringLiteral("filesystem body"));

    WhatSonTimestampConflictResolver::MergeRequest incomingNewer = filesystemNewer;
    incomingNewer.incomingLastModifiedAt = QStringLiteral("2026-05-01-13-00-00");

    const WhatSonTimestampConflictResolver::MergeResult incomingResult =
        resolver.mergeBodyByTimestamp(incomingNewer);
    QVERIFY(incomingResult.conflictDetected);
    QCOMPARE(
        incomingResult.winner,
        QStringLiteral("incoming"));
    QCOMPARE(
        incomingResult.mergedBodySourceText,
        QStringLiteral("incoming body"));
}

void WhatSonCppRegressionTests::timestampConflictResolver_reportsStrictlyNewerTimestamp()
{
    WhatSonTimestampConflictResolver resolver;

    QVERIFY(resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-12-00-00"),
        QStringLiteral("2026-05-01-10-00-00")));
    QVERIFY(!resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-10-00-00"),
        QStringLiteral("2026-05-01-12-00-00")));
    QVERIFY(!resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-12-00-00"),
        QStringLiteral("2026-05-01-12-00-00")));
    QVERIFY(resolver.isTimestampNewer(
        QStringLiteral("2026-05-01-12-00-00"),
        QString()));
    QVERIFY(!resolver.isTimestampNewer(
        QString(),
        QStringLiteral("2026-05-01-12-00-00")));
}

void WhatSonCppRegressionTests::localNoteFileStore_keepsFilesystemBodyWhenTimestampConflictIsNewer()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("filesystem-newer-conflict-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("base body"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteDocument baseDocument;
    QString readError;
    QVERIFY2(fileStore.readNote(readRequest, &baseDocument, &readError), qPrintable(readError));
    const QString baseLastModifiedAt = baseDocument.headerStore.lastModifiedAt();

    WhatSonLocalNoteDocument filesystemDocument = baseDocument;
    filesystemDocument.bodySourceText = QStringLiteral("filesystem body");
    filesystemDocument.bodyPlainText = QStringLiteral("filesystem body");
    WhatSonLocalNoteFileStore::UpdateRequest filesystemUpdate;
    filesystemUpdate.document = filesystemDocument;
    filesystemUpdate.touchLastModified = true;
    filesystemUpdate.incomingLastModifiedAt = QStringLiteral("2026-05-01-12-00-00");

    WhatSonLocalNoteDocument externallyUpdatedDocument;
    QString externalUpdateError;
    QVERIFY2(
        fileStore.updateNote(filesystemUpdate, &externallyUpdatedDocument, &externalUpdateError),
        qPrintable(externalUpdateError));

    WhatSonLocalNoteDocument staleDocument = baseDocument;
    staleDocument.bodySourceText = QStringLiteral("stale incoming body");
    staleDocument.bodyPlainText = QStringLiteral("stale incoming body");

    WhatSonLocalNoteFileStore::UpdateRequest staleUpdate;
    staleUpdate.document = staleDocument;
    staleUpdate.touchLastModified = true;
    staleUpdate.baseLastModifiedAt = baseLastModifiedAt;
    staleUpdate.incomingLastModifiedAt = QStringLiteral("2026-05-01-11-00-00");

    WhatSonLocalNoteDocument resolvedDocument;
    WhatSonLocalNoteFileStore::UpdateResult updateResult;
    QString staleUpdateError;
    QVERIFY2(
        fileStore.updateNote(staleUpdate, &resolvedDocument, &staleUpdateError, &updateResult),
        qPrintable(staleUpdateError));

    QVERIFY(updateResult.conflictResolvedByTimestamp);
    QCOMPARE(updateResult.conflictWinner, QStringLiteral("filesystem"));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(resolvedDocument.bodySourceText),
        QStringLiteral("filesystem body"));
    QCOMPARE(resolvedDocument.headerStore.modifiedCount(), 1);

    WhatSonLocalNoteDocument persistedDocument;
    QVERIFY2(fileStore.readNote(readRequest, &persistedDocument, &readError), qPrintable(readError));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(persistedDocument.bodySourceText),
        QStringLiteral("filesystem body"));
    QCOMPARE(persistedDocument.headerStore.modifiedCount(), 1);
}

void WhatSonCppRegressionTests::localNoteFileStore_acceptsIncomingBodyWhenTimestampConflictIsNewer()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("incoming-newer-conflict-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("base body"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteDocument baseDocument;
    QString readError;
    QVERIFY2(fileStore.readNote(readRequest, &baseDocument, &readError), qPrintable(readError));
    const QString baseLastModifiedAt = baseDocument.headerStore.lastModifiedAt();

    WhatSonLocalNoteDocument filesystemDocument = baseDocument;
    filesystemDocument.bodySourceText = QStringLiteral("filesystem body");
    filesystemDocument.bodyPlainText = QStringLiteral("filesystem body");
    WhatSonLocalNoteFileStore::UpdateRequest filesystemUpdate;
    filesystemUpdate.document = filesystemDocument;
    filesystemUpdate.touchLastModified = true;
    filesystemUpdate.incomingLastModifiedAt = QStringLiteral("2026-05-01-12-00-00");

    WhatSonLocalNoteDocument externallyUpdatedDocument;
    QString externalUpdateError;
    QVERIFY2(
        fileStore.updateNote(filesystemUpdate, &externallyUpdatedDocument, &externalUpdateError),
        qPrintable(externalUpdateError));

    WhatSonLocalNoteDocument incomingDocument = baseDocument;
    incomingDocument.bodySourceText = QStringLiteral("incoming body");
    incomingDocument.bodyPlainText = QStringLiteral("incoming body");

    WhatSonLocalNoteFileStore::UpdateRequest incomingUpdate;
    incomingUpdate.document = incomingDocument;
    incomingUpdate.touchLastModified = true;
    incomingUpdate.baseLastModifiedAt = baseLastModifiedAt;
    incomingUpdate.incomingLastModifiedAt = QStringLiteral("2026-05-01-13-00-00");

    WhatSonLocalNoteDocument resolvedDocument;
    WhatSonLocalNoteFileStore::UpdateResult updateResult;
    QString incomingUpdateError;
    QVERIFY2(
        fileStore.updateNote(incomingUpdate, &resolvedDocument, &incomingUpdateError, &updateResult),
        qPrintable(incomingUpdateError));

    QVERIFY(updateResult.conflictResolvedByTimestamp);
    QCOMPARE(updateResult.conflictWinner, QStringLiteral("incoming"));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(resolvedDocument.bodySourceText),
        QStringLiteral("incoming body"));
    QCOMPARE(resolvedDocument.headerStore.modifiedCount(), 2);
    QCOMPARE(resolvedDocument.headerStore.lastModifiedAt(), QStringLiteral("2026-05-01-13-00-00"));
}
