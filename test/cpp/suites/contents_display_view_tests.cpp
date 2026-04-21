#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsDisplayView_invalidatesGutterGeometryImmediatelyAcrossRapidNoteSwitches()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("property string minimapLineGroupsNoteId: \"\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function activeLineGeometryNoteId()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasPendingNoteEntryGutterRefresh(noteId)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function resetNoteEntryLineGeometryState()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId === contentsView.activeLineGeometryNoteId()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("currentNoteId === contentsView.minimapLineGroupsNoteId")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId = currentNoteId;")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId = \"\";")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredDocumentFlow.scheduleLayoutCacheRefresh();")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleViewportGutterRefresh();")));
    QVERIFY(displayViewSource.contains(QStringLiteral("refreshCoordinator.scheduleNoteEntryGutterRefresh(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("refreshPlan.gutterPassCount")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleGutterRefresh(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("minimapCoordinator.buildNextMinimapSnapshotPlan(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contextMenuCoordinator.openSelectionContextMenuPlan(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("gutterCoordinator.buildVisiblePlainGutterLineEntries(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"structured-layout-cache\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"editor-text-synchronized\"")));
}

void WhatSonCppRegressionTests::contentsDisplayView_keepsGutterNumbersCloseToTheEditorBody()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property int gutterBodyGap")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property int lineNumberRightInset: contentsView.gutterBodyGap")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("readonly property int lineNumberRightInset: contentsView.editorHorizontalInset")));
}

void WhatSonCppRegressionTests::contentsDisplayView_reservesLargeBottomAccessibilityMargin()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property int editorBottomInset: Math.max(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("Math.round(contentsView.editorSurfaceHeight * 0.5)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("insetVertical: contentsView.showPrintEditorLayout ? 0 : contentsView.editorBottomInset")));
    QVERIFY(displayViewSource.contains(QStringLiteral("+ contentsView.editorBottomInset)")));
}

void WhatSonCppRegressionTests::contentsDisplayView_usesSelectedNoteSnapshotWhileSessionBindingCatchesUp()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(
        QStringLiteral("ContentsDisplaySessionCoordinator")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("sessionCoordinator.resolvedDocumentPresentationSourceText()")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? \"\" : String(contentsView.selectedNoteBodyNoteId)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? \"\" : String(contentsView.selectedNoteBodyText)")));
}
