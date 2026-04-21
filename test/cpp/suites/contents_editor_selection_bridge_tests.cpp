#include "../whatson_cpp_regression_tests.hpp"

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
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("Body 2"));
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
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
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
}

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_refreshesSelectedBodyFromNoteListBodySignal()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());

    noteListModel.setCurrentBodyText(QStringLiteral("Late body snapshot"));
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QStringLiteral("Late body snapshot"));
    QVERIFY(!selectionBridge.selectedNoteBodyLoading());
}
