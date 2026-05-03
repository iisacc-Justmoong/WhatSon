#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlInternalTypeRegistrar_usesLvrsManifestRegistration()
{
    const QString registrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));

    QVERIFY(registrarSource.contains(QStringLiteral("lvrs::QmlTypeRegistration")));
    QVERIFY(registrarSource.contains(QStringLiteral("internalQmlTypeRegistrationManifest()")));
    QVERIFY(registrarSource.contains(QStringLiteral("lvrs::registerQmlTypes")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsEditorPresentationProjection")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsStructuredBlockRenderer")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsGutterLayoutMetrics")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsGutterLineNumberGeometry")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsGutterMarkerGeometry")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsMinimapLayoutMetrics")));
}

void WhatSonCppRegressionTests::qmlContextMenus_treatRightClickAndLongPressAsSymmetricPointerTriggers()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("requestEditorSelectionContextMenuFromPointer")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"right-click\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"long-press\"")));
}

void WhatSonCppRegressionTests::qmlHierarchyNoteDrop_keepsDropSurfaceOpenUntilCapabilityRejectsTarget()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(sidebarSource.contains(QStringLiteral("noteDropSurface")));
    QVERIFY(sidebarSource.contains(QStringLiteral("updateNoteDropPreviewAtPosition")));
    QVERIFY(sidebarSource.contains(QStringLiteral("drag.accepted = accepted")));
    QVERIFY(sidebarSource.contains(QStringLiteral("clearNoteDropPreview")));
}

void WhatSonCppRegressionTests::qmlHierarchyExpansion_preservesUserControlledStateAcrossModelRefreshes()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(sidebarSource.contains(QStringLiteral("rememberHierarchyExpansionState")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyExpansionStateForKey")));
    QVERIFY(sidebarSource.contains(QStringLiteral("userExpansionArmed")));
    QVERIFY(sidebarSource.contains(QStringLiteral("refreshEditingHierarchyPresentation")));
}

void WhatSonCppRegressionTests::listBarLayout_rendersResolvedNoteListModelByIndex()
{
    const QString listBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"));

    QVERIFY(listBarSource.contains(QStringLiteral("readonly property var resolvedNoteListModel")));
    QVERIFY(listBarSource.contains(QStringLiteral("function currentIndexFromModel()")));
    QVERIFY(listBarSource.contains(QStringLiteral("function currentNoteEntryFromModel()")));
    QVERIFY(listBarSource.contains(QStringLiteral("function noteIdAtIndex(index)")));
    QVERIFY(listBarSource.contains(QStringLiteral("model.index(normalizedIndex, 0)")));
}

void WhatSonCppRegressionTests::qmlInlineSelectionHelpers_bindOwnersAfterControllerFileDeletion()
{
    const QString listBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"));
    const QString detailContentsSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/DetailContents.qml"));
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(listBarSource.contains(QStringLiteral("property var view: listBarLayout")));
    QVERIFY(detailContentsSource.contains(QStringLiteral("property var section: listSection")));
    QVERIFY(sidebarSource.contains(QStringLiteral("property var view: sidebarHierarchyView")));
    QVERIFY(!listBarSource.contains(QStringLiteral("property var view: null")));
    QVERIFY(!detailContentsSource.contains(QStringLiteral("property var section: null")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("property var view: null")));
    QVERIFY(!listBarSource.contains(QStringLiteral("controller.")));
    QVERIFY(!detailContentsSource.contains(QStringLiteral("controller.")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_consumeRendererNormalizedBlocksWithoutLocalFlattening()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("normalizedHtmlBlocks: editorPresentationProjection.normalizedHtmlBlocks")));
    QVERIFY(displayViewSource.contains(QStringLiteral("logicalText: editorPresentationProjection.logicalText")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function normalizedBlocks()")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string logicalText: \"\"")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("return documentFlow.normalizedHtmlBlocks")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("flatten")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_refreshesGutterLayoutOnEditorOpen()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("scheduleStructuredDocumentOpenLayoutRefresh")));
    QVERIFY(displayViewSource.contains(QStringLiteral("commitDocumentPresentationRefresh")));
    QVERIFY(displayViewSource.contains(QStringLiteral("noteDocumentParseMounted")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_mountsGutterEditorAndMinimapInDisplayLayout()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString gutterSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/Gutter.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("import \"..\" as ContentsChrome")));
    QVERIFY(displayViewSource.contains(QStringLiteral("LV.HStack {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayEditorChromeHStack\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsGutterLayoutMetrics {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsGutterLineNumberGeometry {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsGutterMarkerGeometry {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsMinimapLayoutMetrics {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsChrome.Gutter {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("Flickable {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("id: editorDocumentViewport")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayEditorDocumentViewport\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentHeight: Math.max(height, structuredDocumentFlow.editorContentHeight)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("interactive: contentHeight > height")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function syncSessionFromCurrentNote(resetViewport)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("resetViewport === true && editorDocumentViewport")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onCurrentRawBodyTextChanged: contentsDisplayView.syncSessionFromCurrentNote(false)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onContentYChanged: contentsDisplayView.refreshGutterLineNumberGeometry()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("LV.WheelScrollGuard {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("targetFlickable: editorDocumentViewport")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("property color gutterColor")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("gutterColor: contentsDisplayView.gutterColor")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsStructuredDocumentFlow {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsChrome.Minimap {")));
    const qsizetype hStackIndex = displayViewSource.indexOf(QStringLiteral("LV.HStack {"));
    const qsizetype gutterIndex = displayViewSource.indexOf(QStringLiteral("ContentsChrome.Gutter {"));
    const qsizetype editorIndex = displayViewSource.indexOf(QStringLiteral("ContentsStructuredDocumentFlow {"));
    const qsizetype minimapIndex = displayViewSource.indexOf(QStringLiteral("ContentsChrome.Minimap {"));
    QVERIFY(hStackIndex >= 0);
    QVERIFY(gutterIndex > hStackIndex);
    QVERIFY(editorIndex > gutterIndex);
    QVERIFY(minimapIndex > editorIndex);
    QVERIFY(displayViewSource.contains(QStringLiteral("Layout.preferredWidth: gutterLayoutMetrics.effectiveGutterWidth")));
    QVERIFY(displayViewSource.contains(QStringLiteral("activeLineNumber: gutterMarkerGeometry.cursorLineNumber")));
    QVERIFY(displayViewSource.contains(QStringLiteral("lineNumberColumnLeft: gutterLayoutMetrics.lineNumberColumnLeft")));
    QVERIFY(displayViewSource.contains(QStringLiteral("lineNumberColumnTextWidth: gutterLayoutMetrics.lineNumberColumnTextWidth")));
    QVERIFY(displayViewSource.contains(QStringLiteral("lineNumberEntries: gutterLineNumberGeometry.lineNumberEntries")));
    QVERIFY(displayViewSource.contains(QStringLiteral("markerEntries: gutterMarkerGeometry.markerEntries")));
    QVERIFY(displayViewSource.contains(QStringLiteral("cursorPosition: structuredDocumentFlow.editorCursorPosition")));
    QVERIFY(displayViewSource.contains(QStringLiteral("savedSourceText: contentsDisplayView.currentRawBodyText")));
    QVERIFY(displayViewSource.contains(QStringLiteral("editorGeometryHost: structuredDocumentFlow")));
    QVERIFY(displayViewSource.contains(QStringLiteral("fallbackTopInset: LV.Theme.gapNone")));
    QVERIFY(displayViewSource.contains(QStringLiteral("documentBlocks: structuredBlockRenderer.renderedDocumentBlocks")));
    QVERIFY(displayViewSource.contains(QStringLiteral("renderedResources: bodyResourceRenderer.renderedResources")));
    QVERIFY(displayViewSource.contains(QStringLiteral("logicalLineStartOffsets: editorPresentationProjection.logicalLineStartOffsets")));
    QVERIFY(displayViewSource.contains(QStringLiteral("editorPresentationProjection.logicalToSourceOffsets()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("logicalToSourceOffsets: editorPresentationProjection.logicalToSourceOffsets()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("mapTarget: contentsDisplayGutter")));
    QVERIFY(gutterSource.contains(QStringLiteral("readonly property real lineHeight")));
    QVERIFY(gutterSource.contains(QStringLiteral("Number(modelData.height)")));
    QVERIFY(gutterSource.contains(QStringLiteral("height: lineHeight")));
    QVERIFY(displayViewSource.contains(QStringLiteral("Layout.preferredWidth: minimapLayoutMetrics.effectiveMinimapWidth")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("readonly property int effectiveGutterWidth")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("readonly property int effectiveMinimapWidth")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("anchors.fill: parent\n        editorSurfaceHtml")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_rejectStaleSourceRangeMutations()
{
    const QString mutationPolicySource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/structure/ContentsStructuredDocumentMutationPolicy.cpp"));

    QVERIFY(mutationPolicySource.contains(QStringLiteral("sourceText")));
    QVERIFY(mutationPolicySource.contains(QStringLiteral("sourceStart")));
    QVERIFY(mutationPolicySource.contains(QStringLiteral("sourceEnd")));
    QVERIFY(mutationPolicySource.contains(QStringLiteral("normalizedText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_preserveNativeMobileInputDuringFocusedEdits()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByKeyboard: control.selectByKeyboard")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByMouse: control.selectByMouse")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("persistentSelection: control.persistentSelection")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("InputMethod.")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_commitsPlainTextBlocksDirectlyToRawSource()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("commitEditedSourceText")));
    QVERIFY(displayViewSource.contains(QStringLiteral("commitRawEditorTextMutation")));
    QVERIFY(displayViewSource.contains(QStringLiteral("saveCurrentBodyText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_insertsInlineFormatTagsAtCollapsedCursor()
{
    const QString mutationSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsRawInlineStyleMutationSupport.js"));

    QVERIFY(mutationSupportSource.contains(QStringLiteral("buildInlineStyleSelectionPayload")));
    QVERIFY(mutationSupportSource.contains(QStringLiteral("selectionStart")));
    QVERIFY(mutationSupportSource.contains(QStringLiteral("selectionEnd")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_wrapsSelectedTextIntoRawInlineStyleTags()
{
    const QString mutationSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsRawInlineStyleMutationSupport.js"));

    QVERIFY(mutationSupportSource.contains(QStringLiteral("buildInlineStyleSelectionPayload")));
    QVERIFY(mutationSupportSource.contains(QStringLiteral("unsupported-style-tag")));
    QVERIFY(mutationSupportSource.contains(QStringLiteral("nextSourceText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_insertStructuredShortcutsThroughRawSourceMutations()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("eventRequestsBodyTagShortcut")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("tagManagementKeyPressHandler")));
    QVERIFY(displayViewSource.contains(QStringLiteral("applyDocumentSourceMutation")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_acceptsPlatformCommandModifierForInlineFormatting()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.ControlModifier")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.MetaModifier")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("!optionHeld")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_routesInlineFormatShortcutThroughDocumentFlow()
{
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(documentFlowSource.contains(QStringLiteral("ContentsInlineFormatEditor {")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("onTextEdited: function (text)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("sourceTextEdited(text)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_requireCommittedRawMutationForTagCommands()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("applyDocumentSourceMutation")));
    QVERIFY(displayViewSource.contains(QStringLiteral("commitEditedSourceText")));
    QVERIFY(displayViewSource.contains(QStringLiteral("commitRawEditorTextMutation")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_bindSessionAndFlushTagMutationsToRawPersistence()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsEditorSessionController {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("property var noteActiveState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("noteActiveState.editorSession = editorSession")));
    QVERIFY(displayViewSource.contains(QStringLiteral("syncEditorSessionFromActiveNote")));
    QVERIFY(displayViewSource.contains(QStringLiteral("requestSyncEditorTextFromSelection")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property var noteActiveState")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteActiveState: contentViewLayout.noteActiveState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("saveCurrentBodyText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_pressRightClickRequestsContextMenuAndFocusedBodyTagShortcuts()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("requestEditorSelectionContextMenuFromPointer")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("eventRequestsBodyTagShortcut")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_mapsBottomMarginToTerminalBodyClick()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("terminalBodyClickSourceOffset")));
    QVERIFY(displayViewSource.contains(QStringLiteral("editorSession.editorText.length")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function pointRequestsTerminalBodyClick")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function focusTerminalBodyFromPoint")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("propagateComposedEvents: true")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("height: Math.max(0, documentFlow.height - y)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("terminalBodyClickSurface.y + mouse.y")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("editor.focusTerminalBodyPosition()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function focusTerminalBodyPosition()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.setCursorPositionPreservingNativeInput(textInput.length)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_backspaceDeletesPreviousResourceFromEmptyTextBlock()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("key !== Qt.Key_Backspace")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("tagManagementKeyPressHandler")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_deletesEmptyCalloutWithBackspace()
{
    const QString sessionSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(sessionSource.contains(QStringLiteral("kEmptyCalloutPattern")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.Key_Backspace")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_renderInlineStyleOverlayAtRuntime()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString rendererSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRenderer.cpp"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("property string renderedText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textFormat: TextEdit.RichText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readOnly: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatProjectedCursor\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("cursorDelegate: Rectangle {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeCursorVisible: !control.renderedOverlayVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatNativeCursor\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visible: control.nativeCursorVisible")));
    QVERIFY(rendererSource.contains(QStringLiteral("htmlOverlayVisibleChanged")));
}

void WhatSonCppRegressionTests::qmlEditorInputPolicyAdapter_centralizesNativeInputDecisions()
{
    const QString adapterHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp"));

    QVERIFY(adapterHeader.contains(QStringLiteral("ContentsEditorInputPolicyAdapter")));
    QVERIFY(adapterHeader.contains(QStringLiteral("Q_PROPERTY")));
    QVERIFY(adapterHeader.contains(QStringLiteral("Q_INVOKABLE")));
}

void WhatSonCppRegressionTests::qmlEditorViewDirectory_containsOnlyViewSurfaceFiles()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString bodyLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/BodyLayout.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsDisplayView {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsResourceEditorView {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("property color gutterColor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("gutterColor: contentViewLayout.gutterColor")));
    QVERIFY(!bodyLayoutSource.contains(QStringLiteral("property color gutterColor")));
    QVERIFY(!bodyLayoutSource.contains(QStringLiteral("gutterColor: hStack.gutterColor")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_lockCustomInputToTagManagementOnly()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("editorCustomTextInputEnabled: false")));
    QVERIFY(displayViewSource.contains(QStringLiteral("editorTagManagementInputEnabled: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("tagManagementKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("property bool paperPaletteEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property bool paperPaletteEnabled")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_clipInlineResourceCardsToMeasuredBlockBounds()
{
    const QString resourceViewerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsResourceViewer.qml"));
    const QString resourceEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsResourceEditorView.qml"));

    QVERIFY(resourceViewerSource.contains(QStringLiteral("fillMode: Image.PreserveAspectFit")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("visible: resourceEditor.hasResourceSelection && resourceBitmapState.bitmapRenderable")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_wireInlineResourceRendererToIiXmlHtmlBlockPipeline()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsStructuredBlockRenderer {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsBodyResourceRenderer {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("documentBlocks: structuredBlockRenderer.renderedDocumentBlocks")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsInlineResourcePresentationController {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("renderInlineResourceEditorSurfaceHtml(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("bodyResourceRenderer.renderedResources")));
    QVERIFY(displayViewSource.contains(QStringLiteral("inlineHtmlImageRenderingEnabled: true")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string editorSurfaceHtml")));
}
