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

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromDirectSourceSnapshot()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    FakeContentPersistenceViewModel contentViewModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));
    contentViewModel.setNoteBodySourceText(QStringLiteral("note-1"), QStringLiteral("Prefetched body"));

    selectionBridge.setContentViewModel(&contentViewModel);
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
    FakeContentPersistenceViewModel contentViewModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-empty"));
    contentViewModel.setNoteDirectoryPath(QStringLiteral("note-empty"), QStringLiteral("/tmp/note-empty"));
    contentViewModel.setNoteBodySourceText(QStringLiteral("note-empty"), QString());

    selectionBridge.setContentViewModel(&contentViewModel);
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

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_retainsSelectedNoteAcrossTransientEmptyCurrentNoteId()
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

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
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
}
