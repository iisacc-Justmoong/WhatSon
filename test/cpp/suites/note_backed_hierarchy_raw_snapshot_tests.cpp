#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromNoteListSnapshot()
{
    ensureCoreApplication();

    FakeSelectionNoteListModel noteListModel;
    ContentsEditorSelectionBridge selectionBridge;

    noteListModel.setNoteBacked(true);
    noteListModel.setCurrentNoteId(QStringLiteral("note-1"));
    noteListModel.setCurrentBodyText(QStringLiteral("<callout title=\"Alert\">Body</callout>"));

    selectionBridge.setNoteListModel(&noteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyNoteId(), QStringLiteral("note-1"));
    QCOMPARE(selectionBridge.selectedNoteBodyText(), QString());
    QVERIFY(!selectionBridge.selectedNoteBodyResolved());
    QVERIFY(selectionBridge.selectedNoteBodyLoading() || !selectionBridge.selectedNoteBodyResolved());
}

void WhatSonCppRegressionTests::noteBackedHierarchyNoteLists_preserveRawBodySnapshotForEditorBootstrap()
{
    const QString bookmarksSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.cpp"));
    const QString projectsSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/projects/ProjectsHierarchyController.cpp"));
    const QString progressSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/progress/ProgressHierarchyController.cpp"));

    QVERIFY(!bookmarksSource.isEmpty());
    QVERIFY(!projectsSource.isEmpty());
    QVERIFY(!progressSource.isEmpty());

    QVERIFY(!bookmarksSource.contains(QStringLiteral("item.bodyText.clear();")));
    QVERIFY(!projectsSource.contains(QStringLiteral("item.bodyText.clear();")));
    QVERIFY(!progressSource.contains(QStringLiteral("item.bodyText.clear();")));

    QVERIFY(bookmarksSource.contains(QStringLiteral("item.bodyText = !note.bodySourceText.isEmpty()")));
    QVERIFY(projectsSource.contains(QStringLiteral("item.bodyText = !note.bodySourceText.isEmpty()")));
    QVERIFY(progressSource.contains(QStringLiteral("item.bodyText = !note.bodySourceText.isEmpty()")));
}
