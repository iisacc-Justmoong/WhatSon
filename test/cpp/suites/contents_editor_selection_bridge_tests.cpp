#include "../whatson_cpp_regression_tests.hpp"

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
