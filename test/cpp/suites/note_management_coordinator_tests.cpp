#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteManagementCoordinator_reconcilePersistsEditorSnapshotWhenPreferred()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("authoritative-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("raw-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    FakeContentPersistenceController contentController;
    contentController.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentController(&contentController);

    QSignalSpy reconcileSpy(
        &coordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled);

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::ReconcileViewSessionSnapshot;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("raw-before editor-after");
    request.preferViewSessionOnMismatch = true;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY(result.success);
    QVERIFY(result.viewSessionPersisted);
    QVERIFY(result.snapshotRefreshRequested);

    coordinator.handleRequestFinished(result);
    QCOMPARE(reconcileSpy.count(), 1);

    const QList<QVariant> reconcileArguments = reconcileSpy.takeFirst();
    QCOMPARE(reconcileArguments.at(0).toString(), noteId);
    QVERIFY(reconcileArguments.at(1).toBool());
    QVERIFY(reconcileArguments.at(2).toBool());
    QCOMPARE(reconcileArguments.at(3).toString(), QString());

    QCOMPARE(contentController.applyPersistedBodyStateCallCount, 1);
    QCOMPARE(contentController.lastAppliedNoteId, noteId);
    QCOMPARE(contentController.lastAppliedBodyPlainText, QStringLiteral("raw-before editor-after"));
    QCOMPARE(contentController.lastAppliedBodySourceText, QStringLiteral("raw-before editor-after"));
    QCOMPARE(contentController.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentController.lastReloadedNoteId, noteId);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read reconciled note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("raw-before editor-after"));
    QCOMPARE(document.headerStore.modifiedCount(), 1);
}

void WhatSonCppRegressionTests::noteManagementCoordinator_reconcileRejectsWholeEditorSnapshotReplacement()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("whole-replacement-reconcile-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("raw-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::ReconcileViewSessionSnapshot;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("editor-after");
    request.preferViewSessionOnMismatch = true;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY(result.success);
    QVERIFY(!result.viewSessionPersisted);
    QVERIFY(result.snapshotRefreshRequested);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read reconciled note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("raw-before"));
    QCOMPARE(document.headerStore.modifiedCount(), 0);
}

void WhatSonCppRegressionTests::noteManagementCoordinator_reconcileRefreshesWithoutPersistingWhenEditorIsNotAuthoritative()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("refresh-only-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("raw-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    FakeContentPersistenceController contentController;
    contentController.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentController(&contentController);

    QSignalSpy reconcileSpy(
        &coordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled);

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::ReconcileViewSessionSnapshot;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("editor-after");
    request.preferViewSessionOnMismatch = false;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY(result.success);
    QVERIFY(!result.viewSessionPersisted);
    QVERIFY(result.snapshotRefreshRequested);

    coordinator.handleRequestFinished(result);
    QCOMPARE(reconcileSpy.count(), 1);

    const QList<QVariant> reconcileArguments = reconcileSpy.takeFirst();
    QCOMPARE(reconcileArguments.at(0).toString(), noteId);
    QVERIFY(reconcileArguments.at(1).toBool());
    QVERIFY(reconcileArguments.at(2).toBool());
    QCOMPARE(reconcileArguments.at(3).toString(), QString());

    QCOMPARE(contentController.applyPersistedBodyStateCallCount, 0);
    QCOMPARE(contentController.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentController.lastReloadedNoteId, noteId);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read refreshed note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("raw-before"));
    QCOMPARE(document.headerStore.modifiedCount(), 0);
}

void WhatSonCppRegressionTests::noteManagementCoordinator_directBodyPersistAdvancesModifiedCount()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("direct-edited-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("raw-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::DirectPersistBody;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("raw-after");

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(result.persistedDocument.bodySourceText),
        QStringLiteral("raw-after"));
    QCOMPARE(result.persistedDocument.headerStore.modifiedCount(), 1);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read directly persisted note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("raw-after"));
    QCOMPARE(document.headerStore.modifiedCount(), 1);
}

void WhatSonCppRegressionTests::noteManagementCoordinator_directBodyPersistAppliesEditorDiffToCurrentFilesystemBody()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("direct-diff-edited-note");
    const QString baseSource = QStringLiteral("Alpha\nBeta\nGamma");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        baseSource,
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

    document.bodyPlainText = QStringLiteral("Alpha\nRemote filesystem line\nBeta\nGamma");
    document.bodySourceText = document.bodyPlainText;
    WhatSonLocalNoteFileStore::UpdateRequest remoteUpdate;
    remoteUpdate.document = document;
    remoteUpdate.persistHeader = false;
    remoteUpdate.persistBody = true;
    remoteUpdate.touchLastModified = true;
    remoteUpdate.resolveTimestampConflicts = false;
    QString remoteUpdateError;
    QVERIFY2(
        fileStore.updateNote(remoteUpdate, nullptr, &remoteUpdateError),
        qPrintable(QStringLiteral("Failed to stage remote filesystem body: %1").arg(remoteUpdateError)));

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::DirectPersistBody;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("Alpha\nBeta local\nGamma");
    request.baseBodySourceText = baseSource;
    request.hasBaseBodySourceText = true;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(result.persistedDocument.bodySourceText),
        QStringLiteral("Alpha\nRemote filesystem line\nBeta local\nGamma"));

    WhatSonLocalNoteDocument persistedDocument;
    QString persistedReadError;
    QVERIFY2(
        fileStore.readNote(readRequest, &persistedDocument, &persistedReadError),
        qPrintable(QStringLiteral("Failed to read directly persisted note: %1").arg(persistedReadError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(persistedDocument.bodySourceText),
        QStringLiteral("Alpha\nRemote filesystem line\nBeta local\nGamma"));
    QCOMPARE(persistedDocument.headerStore.modifiedCount(), 2);
}

void WhatSonCppRegressionTests::noteManagementCoordinator_openCountReloadsPersistedMetadata()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("opened-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("open-count body"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    FakeContentPersistenceController contentController;
    contentController.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentController(&contentController);

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::IncrementOpenCount;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY2(result.success, qPrintable(result.errorMessage));

    coordinator.handleRequestFinished(result);
    QCOMPARE(contentController.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentController.lastReloadedNoteId, noteId);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read opened note: %1").arg(readError)));
    QCOMPARE(document.headerStore.openCount(), 1);
    QVERIFY(!document.headerStore.lastOpenedAt().isEmpty());
}

void WhatSonCppRegressionTests::noteManagementCoordinator_loadNoteBodyText_preservesCanonicalSourceText()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("raw-source-note");
    const QString bodySourceText =
        QStringLiteral("<paragraph>Alpha</paragraph>\n<resource type=\"image\" id=\"img-1\" path=\"icloud.wsresources/raw-image.wsresource\" />");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        bodySourceText,
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::LoadNoteBodyText;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read canonicalized note: %1").arg(readError)));

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY(result.success);
    QCOMPARE(result.noteId, noteId);
    QCOMPARE(result.text, document.bodySourceText);
    QVERIFY(result.text.contains(QStringLiteral("<resource ")));
    QVERIFY(result.text.contains(QStringLiteral("Alpha")));
}

void WhatSonCppRegressionTests::noteManagementCoordinator_loadNoteBodyText_prefersExplicitNoteDirectoryPath()
{
    ensureCoreApplication();

    QTemporaryDir workspaceDirA;
    QTemporaryDir workspaceDirB;
    QVERIFY(workspaceDirA.isValid());
    QVERIFY(workspaceDirB.isValid());

    QString createError;
    const QString noteId = QStringLiteral("shared-note-id");
    const QString noteDirectoryPathA = createLocalNoteForRegression(
        workspaceDirA.path(),
        noteId,
        QStringLiteral("Body from path A"),
        &createError);
    QVERIFY2(
        !noteDirectoryPathA.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture A: %1").arg(createError)));

    createError.clear();
    const QString noteDirectoryPathB = createLocalNoteForRegression(
        workspaceDirB.path(),
        noteId,
        QStringLiteral("Body from path B"),
        &createError);
    QVERIFY2(
        !noteDirectoryPathB.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture B: %1").arg(createError)));

    FakeContentPersistenceController contentController;
    contentController.setNoteDirectoryPath(noteId, noteDirectoryPathA);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentController(&contentController);

    QSignalSpy loadSpy(&coordinator, &ContentsNoteManagementCoordinator::noteBodyTextLoaded);

    const quint64 sequence = coordinator.loadNoteBodyTextForNote(noteId, noteDirectoryPathB);
    QVERIFY(sequence != 0);

    QTRY_COMPARE(loadSpy.count(), 1);
    const QList<QVariant> arguments = loadSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<quint64>(), sequence);
    QCOMPARE(arguments.at(1).toString(), noteId);
    QCOMPARE(arguments.at(2).toString(), QStringLiteral("Body from path B"));
    QVERIFY(arguments.at(3).toBool());
    QCOMPARE(arguments.at(4).toString(), QString());
    QVERIFY(!arguments.at(5).toString().trimmed().isEmpty());
}
