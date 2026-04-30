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

    QCOMPARE(contentController.applyPersistedBodyStateCallCount, 1);
    QCOMPARE(contentController.lastAppliedNoteId, noteId);
    QCOMPARE(contentController.lastAppliedBodyPlainText, QStringLiteral("editor-after"));
    QCOMPARE(contentController.lastAppliedBodySourceText, QStringLiteral("editor-after"));
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
}
