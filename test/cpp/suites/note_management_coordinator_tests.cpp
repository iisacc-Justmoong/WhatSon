#include "../whatson_cpp_regression_tests.hpp"

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

    FakeContentPersistenceViewModel contentViewModel;
    contentViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentViewModel(&contentViewModel);

    QSignalSpy reconcileSpy(
        &coordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled);

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::ReconcileViewSessionSnapshot;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("editor-after");
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

    QCOMPARE(contentViewModel.applyPersistedBodyStateCallCount, 1);
    QCOMPARE(contentViewModel.lastAppliedNoteId, noteId);
    QCOMPARE(contentViewModel.lastAppliedBodyPlainText, QStringLiteral("editor-after"));
    QCOMPARE(contentViewModel.lastAppliedBodySourceText, QStringLiteral("editor-after"));
    QCOMPARE(contentViewModel.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentViewModel.lastReloadedNoteId, noteId);

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
        QStringLiteral("editor-after"));
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

    FakeContentPersistenceViewModel contentViewModel;
    contentViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentViewModel(&contentViewModel);

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

    QCOMPARE(contentViewModel.applyPersistedBodyStateCallCount, 0);
    QCOMPARE(contentViewModel.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentViewModel.lastReloadedNoteId, noteId);

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
}
