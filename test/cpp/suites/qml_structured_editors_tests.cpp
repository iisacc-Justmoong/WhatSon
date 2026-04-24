#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaLayer.qml"));
    const QString calloutLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutLayer.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaLayerSource.isEmpty());
    QVERIFY(!calloutLayerSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("paperPaletteEnabled: documentFlow.paperPaletteEnabled")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("paperPaletteEnabled: documentBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(
        QStringLiteral("readonly property color editorTextColor: paperPaletteEnabled ? \"#111111\" : LV.Theme.bodyColor")));
    QVERIFY(textBlockSource.contains(QStringLiteral("paperPaletteEnabled: textBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(QStringLiteral("textColor: textBlock.editorTextColor")));

    QVERIFY(!agendaBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("color: \"#80FFFFFF\"")));
    QVERIFY(agendaBlockSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutBlockSource.contains(
        QStringLiteral("readonly property color bodyTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaLayerSource.contains(QStringLiteral("color: \"#FFFFFF\"")));
    QVERIFY(agendaLayerSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutLayerSource.contains(
        QStringLiteral("readonly property color textColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("paperPaletteEnabled: contentsView.showPrintEditorLayout")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_clipInlineResourceCardsToMeasuredBlockBounds()
{
    const QString resourceBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceBlock.qml"));
    const QString resourceCardSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceRenderCard.qml"));

    QVERIFY(!resourceBlockSource.isEmpty());
    QVERIFY(!resourceCardSource.isEmpty());

    QVERIFY(resourceBlockSource.contains(QStringLiteral("implicitHeight: resourceCard.implicitHeight")));
    QVERIFY(resourceBlockSource.contains(QStringLiteral("clip: true")));
    QVERIFY(resourceCardSource.contains(QStringLiteral("clip: true")));
    QVERIFY(!resourceCardSource.contains(QStringLiteral("clip: false")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_consumeRendererNormalizedBlocksWithoutLocalFlattening()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("documentHost.documentBlocks = documentHost.collectionPolicy.normalizeEntries(documentFlow.documentBlocks)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function flattenedInteractiveBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function normalizedParsedBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function buildFlattenedInteractiveTextGroup(")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function implicitTextBlockInteractiveFlattenCandidate(")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("function snapshotTokenForLogicalLine(blockEntry, logicalLines, lineIndex)")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("\"snapshotToken\": documentFlow.snapshotTokenForLogicalLine(blockEntry, logicalLines, index)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_rejectStaleSourceRangeMutations()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function sourceRangeMatchesCurrentSnapshot(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function replaceSourceRangeIfCurrent(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("sourceRangeMatchesCurrentSnapshot.rejected")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("safeBlock.sourceText")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"text-block\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("const expectedTaskSourceText = StructuredCursorSupport.replacementSourceText(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"agenda-task\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("const expectedCalloutSourceText = StructuredCursorSupport.replacementSourceText(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"callout-text\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"allowBlankAnchor\": true")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_preserveNativeMobileInputDuringFocusedEdits()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());

    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool nativeTextInputPriority: contentsView.mobileHost || Qt.platform.os === \"ios\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function nativeTextInputSessionOwnsKeyboard()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredDocumentFlow.nativeCompositionActive()")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("context: Qt.WindowShortcut\n                        enabled: contentsView.noteDocumentCommandSurfaceEnabled")));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool _localTextEditSinceFocus: false")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("if (documentFlow.nativeTextInputPriority)\n            return")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("expectedPreviousSourceText")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("sourceStart + expectedSourceText.length")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("expectedPreviousText")));

    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("readonly property bool inputMethodComposing")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("readonly property string preeditText")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("nativeTextInputPriority: documentBlock.nativeTextInputPriority")));

    QVERIFY(textBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(textBlockSource.contains(QStringLiteral("property string _liveEditSourceText: \"\"")));
    QVERIFY(textBlockSource.contains(QStringLiteral("syncLiveEditSnapshotFromHost")));
    QVERIFY(textBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(agendaBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("property string _liveTaskText: \"\"")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("syncLiveTaskTextFromHost")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("expectedPreviousText")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(calloutBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("property string _liveText: \"\"")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("syncLiveTextFromHost")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("expectedPreviousText")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_lockCustomInputToTagManagementOnly()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString breakBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsBreakBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString selectionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsEditorSelectionController.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsEditorTypingController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!breakBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!selectionControllerSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool editorCustomTextInputEnabled: false")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool editorTagManagementInputEnabled: true")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function handleTagManagementShortcutKeyPress(event)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("tagManagementShortcutKeyPressHandler: function (event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function handleInlineFormatShortcutKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("handlePlainEnterKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("handleTagAwareDeleteKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("queueMarkdownListMutation")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Meta+Shift+7\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Alt+Shift+7\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Meta+Shift+8\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Alt+Shift+8\"")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function blockEntryIsTagManagedAtomicBlock(blockEntry)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function handleActiveTagManagementKeyPress(event)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function handleActiveBlockDeleteKeyPress(event)")));

    QVERIFY(documentBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("function handleAtomicTagManagementKeyPress(event)")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("function handleAtomicTagDeleteKeyPress(event)")));
    QVERIFY(!documentBlockSource.contains(QStringLiteral("return !!blockItem.handleDeleteKeyPress(event)")));

    QVERIFY(breakBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));

    QVERIFY(!textBlockSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function handleDeleteKeyPress(event)")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function handleAtomicBlockBoundaryKeyPress(event)")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("function handleBoundaryKeyPress(event)")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("function handleBoundaryKeyPress(event)")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));

    QVERIFY(!selectionControllerSource.contains(QStringLiteral("handleInlineFormatShortcutKeyPress")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queueStructuredShortcutMutation")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queuedStructuredShortcutMutationKeys")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queueMarkdownListMutation")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queuedMarkdownListMutationKeys")));

    QVERIFY(!typingControllerSource.contains(QStringLiteral("handlePlainEnterKeyPress")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("handleTagAwareDeleteKeyPress")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("applyDirectRawSourceReplacement")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("continuedListInsertion")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("markerOnlyListBreakInsertion")));
}
