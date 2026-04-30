#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_tracksSelectionFromCurrentIndexSignal()
{
    ensureCoreApplication();

    FakeIndexDrivenSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setEntry(0, QStringLiteral("note-1"), QStringLiteral("Body 1"));
    noteListModel.setEntry(1, QStringLiteral("note-2"), QStringLiteral("Body 2"));

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    noteListModel.setCurrentIndex(1);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-2"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-2"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
    QVERIFY(selectionBridge.selectedNoteBodyLoading() || !selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_preservesNoSelectionSentinelBeforeIndexCommit()
{
    ensureCoreApplication();

    FakeIndexDrivenSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setEntry(0, QStringLiteral("note-1"), QStringLiteral("Body 1"));
    noteListModel.setEntry(1, QStringLiteral("note-2"), QStringLiteral("Body 2"));
    noteListModel.setCurrentIndex(-1);

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(noteListModel.currentIndex(), -1);
    QCOMPARE(noteListModel.itemCount(), 2);
    QCOMPARE(selectionBridge.visibleNoteCount(), 2);
    QCOMPARE(selectionBridge.selectedNoteId(), QString());
    QCOMPARE(selectionBridge.selectedNoteDirectoryPath(), QString());
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QString());
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_requiresCommittedSelectionContractForNoteIdentity()
{
    ensureCoreApplication();

    FakeRowOnlySelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.appendEntry(
        QStringLiteral("note-row-only"),
        QStringLiteral("/tmp/note-row-only.wsnote"),
        QStringLiteral("Body 1"));
    noteListModel.setCurrentIndex(0);

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(noteListModel.currentIndex(), 0);
    QCOMPARE(selectionBridge.visibleNoteCount(), 1);
    QVERIFY(selectionBridge.noteSelectionContractAvailable());
    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-row-only"));
    QCOMPARE(selectionBridge.selectedNoteDirectoryPath(), QStringLiteral("/tmp/note-row-only.wsnote"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QString());
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
    QVERIFY(selectionBridge.selectedNoteBodyLoading() || !selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromDirectSourceSnapshot()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    FakeContentPersistenceController contentController;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));
    contentController.setNoteBodySourceText(QStringLiteral("note-1"), QStringLiteral("Prefetched body"));

    selectionBridge.setContentController(&contentController);
    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("Prefetched body"));
    QVERIFY(selectionBridge.selectedNoteBodyResolved());
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_treatsDirectEmptySourceAsResolvedEmptyNote()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    FakeContentPersistenceController contentController;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-empty"));
    contentController.setNoteDirectoryPath(QStringLiteral("note-empty"), QStringLiteral("/tmp/note-empty"));
    contentController.setNoteBodySourceText(QStringLiteral("note-empty"), QString());

    selectionBridge.setContentController(&contentController);
    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-empty"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-empty"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
    QVERIFY(selectionBridge.selectedNoteBodyLoading() || !selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_ignoresNoteListBodySnapshotWithoutDirectSource()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());

    noteListModel.setCurrentBodyText(QStringLiteral("Late body snapshot"));
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_rebindsSameNoteIdWhenPackagePathChanges()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));
    noteListModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/wsnote-a"));

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteDirectoryPath(), QStringLiteral("/tmp/wsnote-a"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());

    noteListModel.setCurrentNoteDirectoryPath(QStringLiteral("/tmp/wsnote-b"));
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteDirectoryPath(), QStringLiteral("/tmp/wsnote-b"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_clearsSelectedNoteAcrossTransientEmptyCurrentNoteId()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setItemCount(3);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());

    noteListModel.setCurrentNoteId(QString());
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QString());
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QString());
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_reloadsBodyWhenCommittedNoteEntryChangesWithoutNoteIdChange()
{
    ensureCoreApplication();

    FakeCurrentNoteEntryOnlyListModel noteListModel;
    FakeContentPersistenceController contentController;
    ContentsEditorSelectionBridge selectionBridge;

    QVariantMap currentNoteEntry;
    currentNoteEntry.insert(QStringLiteral("noteId"), QStringLiteral("note-1"));
    currentNoteEntry.insert(QStringLiteral("primaryText"), QStringLiteral("First note"));
    noteListModel.setCurrentNoteEntry(currentNoteEntry);
    contentController.setNoteBodySourceText(QStringLiteral("note-1"), QStringLiteral("Body 1"));

    selectionBridge.setContentController(&contentController);
    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("Body 1"));
    QVERIFY(selectionBridge.selectedNoteBodyResolved());

    contentController.setNoteBodySourceText(QStringLiteral("note-1"), QStringLiteral("Body 2"));
    currentNoteEntry.insert(QStringLiteral("primaryText"), QStringLiteral("Renamed note"));
    noteListModel.setCurrentNoteEntry(currentNoteEntry);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("Body 2"));
    QVERIFY(selectionBridge.selectedNoteBodyResolved());
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_updatesBodySnapshotBeforePersistenceFinishedSignal()
{
    ensureCoreApplication();

    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString errorMessage;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("note-1"),
        QStringLiteral("Old body"),
        &errorMessage);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(errorMessage));

    FakeSelectionNoteListModel noteListModel;
    FakeContentPersistenceController contentController;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));
    noteListModel.setCurrentNoteDirectoryPath(noteDirectoryPath);
    contentController.setNoteDirectoryPath(QStringLiteral("note-1"), noteDirectoryPath);
    contentController.setNoteBodySourceText(QStringLiteral("note-1"), QStringLiteral("Old body"));

    selectionBridge.setContentController(&contentController);
    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QTRY_COMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("Old body"));
    QVERIFY(selectionBridge.selectedNoteBodyResolved());

    QSignalSpy persistenceFinishedSpy(
        &selectionBridge,
        &ContentsEditorSelectionBridge::editorTextPersistenceFinished);
    QString selectedBodyAtPersistenceFinished;
    bool persistenceSucceededAtSignal = false;
    QObject::connect(
        &selectionBridge,
        &ContentsEditorSelectionBridge::editorTextPersistenceFinished,
        &selectionBridge,
        [&](const QString& noteId, const QString& text, bool success, const QString&)
        {
            if (noteId == QStringLiteral("note-1") && text == QStringLiteral("New body"))
            {
                selectedBodyAtPersistenceFinished = selectionBridge.selectedNoteBodyText();
                persistenceSucceededAtSignal = success;
            }
        });

    QVERIFY(selectionBridge.flushEditorTextForNote(QStringLiteral("note-1"), QStringLiteral("New body")));

    QTRY_COMPARE(persistenceFinishedSpy.count(), 1);
    QVERIFY(persistenceSucceededAtSignal);
    QCOMPARE(selectedBodyAtPersistenceFinished, QStringLiteral("New body"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("New body"));
    QVERIFY(selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_emitsTraceForNoteSelectionFlow()
{
    const QString selectionBridgeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp"));

    QVERIFY(!selectionBridgeSource.isEmpty());
    QVERIFY(selectionBridgeSource.contains(QStringLiteral("summarizeTraceNoteListModel")));
    QVERIFY(selectionBridgeSource.contains(QStringLiteral("currentBodyText={%6}")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteListSelectionChanged\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteListEntrySelectionChanged\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteListBodyTextChanged\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.refreshScheduled\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.refreshFlush\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.refreshState\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteStable\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteChanged\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteCleared\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadStart\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadImmediate\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadAsyncRequested\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadFromPendingEditor\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadFallbackResolved\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadFallbackEmpty\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.bodyLoadFinished\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.viewSessionSnapshotReconciled\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.refreshSelectedNoteSnapshot\")")));
    QVERIFY(!selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.noteIdRetained\")")));
    QVERIFY(selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.resolveCurrentNoteId.rowFallback\")")));
    QVERIFY(!selectionBridgeSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.resolveCurrentNoteId.readNoteIdAt\")")));
}
