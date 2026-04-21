#include "test/cpp/whatson_cpp_regression_tests.hpp"

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
        QStringLiteral("selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? \"\" : String(contentsView.selectedNoteBodyText)")));
}

void WhatSonCppRegressionTests::contentsDisplayView_surfacesMountFailurePlaceholderWithoutChrome()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayNoteBodyMountCoordinator")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool selectedNoteBodyResolved: selectionBridge.selectedNoteBodyResolved")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool structuredDocumentFlowRequested: contentsView.hasSelectedNote")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool legacyInlineEditorRequested: contentsView.hasSelectedNote")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteSnapshotRefreshEnabled: contentsView.visible && contentsView.hasSelectedNote && !contentsView.selectedNoteBodyLoading && (contentsView.editorSessionBoundToSelectedNote || contentsView.selectedNoteBodyResolved)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMountPending: noteBodyMountCoordinator.mountPending")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("inlineDocumentSurfaceRequested: contentsView.legacyInlineEditorRequested")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? \"\" : String(contentsView.selectedNoteBodyText)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("inlineDocumentSurfaceReady: contentEditorLoader.status === Loader.Ready && contentEditorLoader.item !== null")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("structuredDocumentSurfaceRequested: contentsView.structuredDocumentFlowRequested")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("structuredDocumentSurfaceReady: contentsView.structuredDocumentFlowRequested && structuredDocumentFlow.visible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMounted: noteBodyMountCoordinator.noteMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMountFailureVisible: noteBodyMountCoordinator.mountFailed")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentMountFailureMessage: noteBodyMountCoordinator.mountFailureMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentSurfaceVisible: noteBodyMountCoordinator.surfaceVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentCommandSurfaceEnabled: contentsView.noteDocumentMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("noteBodyMountCoordinator.scheduleMount(options && typeof options === \"object\" ? options : ({}));")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function onMountFlushRequested(plan)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("mountPlan.attemptSnapshotRefresh")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("noteBodyMountCoordinator.handleSnapshotRefreshFinished(")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("const refreshedNoteId = String(mountPlan.selectedNoteId || \"\").trim();")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("if (!contentsView.selectedNoteBodyLoading")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("mountPlan.attemptEditorSessionMount")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("mountPlan.fallbackRefreshIfMountSkipped")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.hasSelectedNote && contentsView.noteDocumentSurfaceVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.showEditorGutter && contentsView.noteDocumentMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.noteDocumentMountPending")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.noteDocumentMountFailureVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("text: contentsView.noteDocumentMountFailureMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("text: \"Loading note...\"")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("onSelectedNoteBodyLoadingChanged:")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("if (contentsView.selectedNoteBodyLoading)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("if (contentsView.selectedNoteId.length === 0)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("onSelectedNoteBodyResolvedChanged:")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("if (!contentsView.selectedNoteBodyResolved)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function focusEditorForSelectedNoteId(noteId)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.showMinimapRail && contentsView.noteDocumentMounted")));
}
