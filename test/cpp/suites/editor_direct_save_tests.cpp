#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorSaveCoordinator_writesDirectlyThroughNoteManagement()
{
    const QString saveCoordinatorHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSaveCoordinator.hpp"));
    const QString saveCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSaveCoordinator.cpp"));
    const QString deletedPersistenceHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/persistence/ContentsEditorPersistenceController.hpp"));
    const QString deletedPersistenceSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/persistence/ContentsEditorPersistenceController.cpp"));
    const QString oldIdleSyncSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/ContentsEditorIdleSyncController.cpp"));
    const QString selectionBridgeHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/bridge/ContentsEditorSelectionBridge.hpp"));
    const QString selectionBridgeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp"));
    const QString sessionSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    const QString testCMakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));

    QVERIFY(deletedPersistenceHeader.isEmpty());
    QVERIFY(deletedPersistenceSource.isEmpty());
    QVERIFY(oldIdleSyncSource.isEmpty());

    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("class ContentsEditorSaveCoordinator final : public QObject")));
    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("Q_INVOKABLE bool commitEditedSourceText")));
    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("Q_INVOKABLE bool syncEditorSessionFromSelection")));
    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("ContentsNoteManagementCoordinator* m_noteManagementCoordinator")));
    QVERIFY(saveCoordinatorSource.contains(QStringLiteral("new ContentsNoteManagementCoordinator(this)")));
    QVERIFY(saveCoordinatorSource.contains(QStringLiteral("persistEditorTextForNoteAtPath")));
    QVERIFY(saveCoordinatorSource.contains(QStringLiteral("captureDirectPersistenceContextForNote")));
    QVERIFY(!saveCoordinatorSource.contains(QStringLiteral("persistEditorTextForNote(noteId, text)")));
    QVERIFY(saveCoordinatorSource.contains(QStringLiteral("flushPendingEditorText")));

    QVERIFY(selectionBridgeHeader.contains(QStringLiteral("class ContentsNoteManagementCoordinator;")));
    QVERIFY(selectionBridgeHeader.contains(QStringLiteral(
        "QPointer<ContentsNoteManagementCoordinator> m_noteManagementCoordinator;")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("ContentsEditorIdleSyncController")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("ContentsEditorPersistenceController")));
    QVERIFY(selectionBridgeSource.contains(QStringLiteral(
        "#include \"app/models/file/note/ContentsNoteManagementCoordinator.hpp\"")));
    QVERIFY(selectionBridgeSource.contains(QStringLiteral("new ContentsNoteManagementCoordinator(this)")));
    QVERIFY(!selectionBridgeSource.contains(QStringLiteral("ContentsEditorPersistenceController")));
    QVERIFY(!selectionBridgeSource.contains(QStringLiteral("bodyLoadFromPendingEditor")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("flushEditorTextForNote")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("stageEditorTextForIdleSync")));
    QVERIFY(!selectionBridgeSource.contains(QStringLiteral("models/file/sync/ContentsEditorIdleSyncController")));
    QVERIFY(!selectionBridgeSource.contains(QStringLiteral("m_idleSyncController")));

    QVERIFY(!sessionSource.contains(QStringLiteral("stageEditorTextForIdleSync")));
    QVERIFY(!sessionSource.contains(QStringLiteral("flushEditorTextForNote")));
    QVERIFY(!sessionSource.contains(QStringLiteral("ContentsNoteManagementCoordinator")));

    QVERIFY(testCMakeSource.contains(QStringLiteral(
        "src/app/models/editor/session/ContentsEditorSaveCoordinator.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral(
        "src/app/models/editor/persistence/ContentsEditorPersistenceController.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral(
        "src/app/models/editor/persistence/ContentsEditorSaveCoordinator.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral(
        "src/app/models/file/sync/ContentsEditorIdleSyncController.cpp")));

    ensureCoreApplication();

    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("direct-save-coordinator-note");
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

    ContentsEditorSessionController editorSession;
    QVERIFY(editorSession.requestSyncEditorTextFromSelection(
        noteId,
        QStringLiteral("raw-before"),
        noteId,
        noteDirectoryPath));

    ContentsEditorSaveCoordinator saveCoordinator;
    saveCoordinator.setContentController(&contentController);
    saveCoordinator.setEditorSession(&editorSession);

    QVERIFY(saveCoordinator.commitEditedSourceText(QStringLiteral("raw-after")));
    QTRY_VERIFY(!editorSession.pendingBodySave());
    QTRY_COMPARE(contentController.applyPersistedBodyStateCallCount, 1);
    QCOMPARE(contentController.lastAppliedNoteId, noteId);
    QCOMPARE(contentController.lastAppliedBodySourceText, QStringLiteral("raw-after"));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read directly saved note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("raw-after"));
    QCOMPARE(document.headerStore.modifiedCount(), 1);
}
