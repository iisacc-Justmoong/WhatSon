#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp"

void WhatSonCppRegressionTests::qmlInternalTypeRegistrar_usesLvrsManifestRegistration()
{
    const QString registrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));

    QVERIFY(registrarSource.contains(QStringLiteral("lvrs::QmlTypeRegistration")));
    QVERIFY(registrarSource.contains(QStringLiteral("internalQmlTypeRegistrationManifest()")));
    QVERIFY(registrarSource.contains(QStringLiteral("lvrs::registerQmlTypes")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsEditorPresentationProjection")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsStructuredBlockRenderer")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsMinimapLayoutMetrics")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsEditorVisualLineMetrics")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsEditorGeometryProvider")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsLineNumberRailMetrics")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsEditorTagInsertionController")));
    QVERIFY(registrarSource.contains(QStringLiteral("ContentsEditorDisplayBackend")));
}

void WhatSonCppRegressionTests::qmlContextMenus_treatRightClickAndLongPressAsSymmetricPointerTriggers()
{
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));

    QVERIFY(displayBackendSource.contains(QStringLiteral("requestEditorSelectionContextMenuFromPointer")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("\"right-click\"")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("\"long-press\"")));
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
    QVERIFY(sidebarSource.contains(QStringLiteral("readonly property var hierarchyTreeContextMenuItems")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyContextMenuKind === \"folder\" ? sidebarHierarchyView.hierarchyFolderContextMenuItems : sidebarHierarchyView.hierarchyTreeContextMenuItems")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("hierarchyViewOptionsMenuItems")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"label\": \"Expand All\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"label\": \"Collapse All\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"eventName\": \"hierarchy.expandAll\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"eventName\": \"hierarchy.collapseAll\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyInteractionBridge.setAllItemsExpanded(true)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyInteractionBridge.setAllItemsExpanded(false)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchy.contextMenu.expandAll")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchy.contextMenu.collapseAll")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function requestHierarchyChevronExpansionAtPosition(x, y, expectedKey)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyInteractionBridge.setItemExpanded(target.index, nextExpanded)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("if (nextState[key] !== undefined)\n                continue;")));
    QVERIFY(sidebarSource.contains(QStringLiteral("onTapped: function (eventPoint, button)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyChevronExpansionAtPosition(tapX, tapY, armedKey);")));
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
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("normalizedHtmlBlocks: editorDisplayBackend.presentationProjection.normalizedHtmlBlocks")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("logicalText: editorDisplayBackend.presentationProjection.logicalText")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function normalizedBlocks()")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string logicalText: \"\"")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("return documentFlow.normalizedHtmlBlocks")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("flatten")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_refreshesDocumentProjectionOnEditorOpen()
{
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));

    QVERIFY(displayBackendHeader.contains(QStringLiteral("commitDocumentPresentationRefresh")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("syncProjectionInputs")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("noteDocumentParseMounted")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_mountsEditorAndMinimapInDisplayLayout()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));
    const QString lineNumberRailSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsLineNumberRail.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("import \"../contents\" as ContentsChrome")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsEditorDisplayBackend {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("LV.HStack {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("objectName: \"contentsDisplayEditorChromeHStack\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property int contentVerticalPadding: LV.Theme.gap8")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("anchors.bottomMargin: contentsEditorSurface.contentVerticalPadding")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("anchors.topMargin: contentsEditorSurface.contentVerticalPadding")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("ContentsMinimapLayoutMetrics")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("Flickable {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("id: editorDocumentViewport")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("objectName: \"contentsDisplayEditorDocumentViewport\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentHeight: editorDocumentContent.height")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("id: editorDocumentContent")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("height: Math.max(editorDocumentViewport.height, structuredDocumentFlow.editorContentHeight)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("interactive: !structuredDocumentFlow.editorRenderedOverlayVisible && contentHeight > height")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function editorViewportScrollRange()")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function scrollEditorViewportByDelta(deltaY)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("const nextContentY = editorDocumentViewport.contentY + (Number(deltaY) || 0)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorDocumentViewport.contentY = Math.max(0, Math.min(scrollRange, nextContentY))")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("syncSessionFromCurrentNote")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("emit editorViewportResetRequested();")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("connectPreserve(\"currentBodyText\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("LV.WheelScrollGuard {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("targetFlickable: editorDocumentViewport")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsLineNumberRail {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("objectName: \"contentsDisplayGutter\"")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property int editorSelectionEnd: editor.selectionEnd")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property int editorSelectionStart: editor.selectionStart")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property bool editorRenderedOverlayVisible: editor.renderedOverlayVisible")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("activeSelectionEnd: structuredDocumentFlow.editorSelectionEnd")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("activeSelectionStart: structuredDocumentFlow.editorSelectionStart")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("activeSourceCursorPosition: structuredDocumentFlow.editorCursorPosition")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("rows: structuredDocumentFlow.editorLogicalGutterRows")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("width: contentsDisplayGutter.preferredWidth")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("property int activeSourceCursorPosition: 0")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("property int activeSelectionEnd: activeSourceCursorPosition")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("property int activeSelectionStart: activeSourceCursorPosition")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("property color activeIndicatorColor: LV.Theme.accentBlue")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property real activeIndicatorWidth: Math.max(LV.Theme.strokeThin, LV.Theme.gap2)")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property real numberColumnWidth: Math.max(LV.Theme.gap16, Number(LV.Theme.textCaption) * 2)")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property real numberRightPadding: LV.Theme.gap8")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property real baselineLeftBlankWidth: Math.max(0, LV.Theme.buttonMinWidth - lineNumberRail.numberColumnWidth - lineNumberRail.numberRightPadding)")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property real leftBlankWidth: lineNumberRail.baselineLeftBlankWidth / 2")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property real preferredWidth: lineNumberRail.leftBlankWidth + lineNumberRail.numberColumnWidth + lineNumberRail.numberRightPadding")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("function rowContainsActiveSourceRange(row)")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("return activeStart < rowEnd && activeEnd > rowStart")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("return cursorPosition >= rowStart && cursorPosition <= rowEnd")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("readonly property bool activeSourceRow: lineNumberRail.rowContainsActiveSourceRange(modelData)")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("color: lineNumberRail.activeIndicatorColor")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("visible: lineNumberRailRow.activeSourceRow")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("anchors.leftMargin: lineNumberRail.leftBlankWidth")));
    QVERIFY(lineNumberRailSource.contains(QStringLiteral("anchors.rightMargin: lineNumberRail.numberRightPadding")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsStructuredDocumentFlow {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsChrome.Minimap {")));
    const qsizetype hStackIndex = contentViewLayoutSource.indexOf(QStringLiteral("LV.HStack {"));
    const qsizetype gutterIndex = contentViewLayoutSource.indexOf(QStringLiteral("ContentsLineNumberRail {"));
    const qsizetype editorIndex = contentViewLayoutSource.indexOf(QStringLiteral("ContentsStructuredDocumentFlow {"));
    const qsizetype minimapIndex = contentViewLayoutSource.indexOf(QStringLiteral("ContentsChrome.Minimap {"));
    QVERIFY(hStackIndex >= 0);
    QVERIFY(gutterIndex > hStackIndex);
    QVERIFY(editorIndex > hStackIndex);
    QVERIFY(editorIndex > gutterIndex);
    QVERIFY(minimapIndex > editorIndex);
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_bodyResourceRenderer.setDocumentBlocks(m_structuredBlockRenderer.renderedDocumentBlocks())")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("Layout.preferredWidth: editorDisplayBackend.minimapLayoutMetrics.effectiveMinimapWidth")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("scrollDragEnabled: editorDocumentViewport.contentHeight > editorDocumentViewport.height")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("onScrollDeltaRequested: function (deltaY)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsEditorSurface.scrollEditorViewportByDelta(deltaY)")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("scrollEditorViewportToRatio")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("onScrollRatioRequested")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("normalizedRatio")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorViewportScrollRatio")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorViewportVisibleRatio")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("scrollPositionRatio:")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("viewportRatio:")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("readonly property int effectiveMinimapWidth")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("anchors.fill: parent\n        editorSurfaceHtml")));
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
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("TapHandler {")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("DragHandler {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.accepted = false;")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("InputMethod.")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_commitsPlainTextBlocksDirectlyToRawSource()
{
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));

    QVERIFY(displayBackendSource.contains(QStringLiteral("commitEditedSourceText")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_editorSaveCoordinator.commitEditedSourceText")));
    QVERIFY(!displayBackendSource.contains(QStringLiteral("saveCurrentBodyText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_insertsInlineFormatTagsAtCollapsedCursor()
{
    const QString tagInsertionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp"));

    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("buildTagInsertionPayload")));
    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("openingTag + closingTag")));
    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("canonicalSourceForGeneratedBodyTag")));
    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("cursorPosition")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_wrapsSelectedTextIntoRawInlineStyleTags()
{
    const QString tagInsertionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp"));

    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("buildTagInsertionPayload")));
    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("source.left(start) + fullReplacementSourceText + source.mid(end)")));
    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("unsupported-tag-kind")));
    QVERIFY(tagInsertionControllerSource.contains(QStringLiteral("nextSourceText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_insertStructuredShortcutsThroughRawSourceMutations()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("eventRequestsBodyTagShortcut")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("tagManagementKeyPressHandler")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("applyDocumentSourceMutation")));
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
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(documentFlowSource.contains(QStringLiteral("ContentsEditorTagInsertionController {")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("ContentsInlineFormatEditor {")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("tagManagementKeyPressHandler: function (event)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function applyInlineFormatShortcut(event)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function applyBodyTagShortcut(event)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("buildTagInsertionPayload")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("tagNameForBodyShortcutKey")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("editor.applyTagManagementMutationPayload(payload)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("onTextEdited: function (text)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("sourceTextEdited(text)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function applyTagManagementMutationPayload(payload)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("applyImmediateProgrammaticText(nextText)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_requireCommittedRawMutationForTagCommands()
{
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));

    QVERIFY(displayBackendSource.contains(QStringLiteral("applyDocumentSourceMutation")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("commitEditedSourceText")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_editorSaveCoordinator.commitEditedSourceText")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_bindSessionAndFlushTagMutationsToRawPersistence()
{
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(displayBackendHeader.contains(QStringLiteral("ContentsEditorSessionController m_editorSession")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("Q_PROPERTY(QObject* noteActiveState")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("attachEditorSessionToActiveState")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("syncEditorSessionFromActiveNote")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_editorSaveCoordinator.syncEditorSessionFromSelection")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("handlePreserveCurrentNoteFromActiveState")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("connectPropertyNotify(m_noteActiveState, \"activeNoteId\"")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("connectPropertyNotify(m_noteActiveState, \"activeNoteBodyText\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceText: editorDisplayBackend.editorSession.editorText")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_bodyResourceRenderer.setDocumentBlocks(m_structuredBlockRenderer.renderedDocumentBlocks())")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property: \"visualLineCount\"")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("logicalLineCount: editorDisplayBackend.presentationProjection.logicalLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property var noteActiveState")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteActiveState: contentViewLayout.noteActiveState")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("editorSaveCoordinator")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_pressRightClickRequestsContextMenuAndFocusedBodyTagShortcuts()
{
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(displayBackendHeader.contains(QStringLiteral("requestEditorSelectionContextMenuFromPointer")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("eventRequestsBodyTagShortcut")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_mapsBottomMarginToTerminalBodyClick()
{
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(displayBackendSource.contains(QStringLiteral("terminalBodyClickSourceOffset")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_editorSession.editorText().size()")));
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

    ContentsEditorInputPolicyAdapter adapter;
    const QVariantMap selectionPolicy = adapter.programmaticTextSyncPolicy(
        QStringLiteral("Alpha"),
        QStringLiteral("Beta"),
        false,
        true,
        true,
        false,
        true);
    QVERIFY(!selectionPolicy.value(QStringLiteral("apply")).toBool());
    QVERIFY(selectionPolicy.value(QStringLiteral("defer")).toBool());
    QVERIFY(adapter.shouldDeferProgrammaticTextSync(
        QStringLiteral("Alpha"),
        QStringLiteral("Beta"),
        false,
        true,
        true,
        false,
        true));
    QVERIFY(!adapter.shouldApplyProgrammaticTextSync(
        QStringLiteral("Alpha"),
        QStringLiteral("Beta"),
        false,
        true,
        true,
        false,
        true));
}

void WhatSonCppRegressionTests::qmlEditorViewDirectory_containsOnlyViewSurfaceFiles()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString bodyLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/BodyLayout.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsEditorDisplayBackend {")));
    QVERIFY(!QFileInfo(QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml")).exists());
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsResourceEditorView {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsEditorSurfaceModeSupport {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsEditorSurfaceModeSupport.js")));
    Q_UNUSED(bodyLayoutSource);

    QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();
    const QDir modelsDirectory(repositoryRoot.filePath(QStringLiteral("src/app/models")));
    QVERIFY2(modelsDirectory.exists(), qPrintable(modelsDirectory.absolutePath()));
    QDirIterator modelScriptFiles(
        modelsDirectory.absolutePath(),
        QStringList{QStringLiteral("*.qml"), QStringLiteral("*.js")},
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);
    QVERIFY2(
        !modelScriptFiles.hasNext(),
        qPrintable(QStringLiteral("Model layer must not contain QML/JS helpers: %1").arg(modelScriptFiles.next())));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_lockCustomInputToTagManagementOnly()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool preferNativeInputHandling: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("tagManagementKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(displayBackendHeader.contains(QStringLiteral("Q_PROPERTY(bool paperPaletteEnabled")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_presentationProjection.setPaperPaletteEnabled(value)")));
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
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString displayBackendHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.hpp"));
    const QString displayBackendSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsEditorDisplayBackend.cpp"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(displayBackendHeader.contains(QStringLiteral("ContentsStructuredBlockRenderer m_structuredBlockRenderer")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("ContentsBodyResourceRenderer m_bodyResourceRenderer")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("m_bodyResourceRenderer.setDocumentBlocks(m_structuredBlockRenderer.renderedDocumentBlocks())")));
    QVERIFY(displayBackendHeader.contains(QStringLiteral("ContentsInlineResourcePresentationController m_inlineResourcePresentation")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("renderInlineResourceEditorSurfaceHtml(")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorDisplayBackend.bodyResourceRenderer.renderedResources")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("targetFrameWidth")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("structuredDocumentFlow.width - LV.Theme.gap16 * 2")));
    QVERIFY(displayBackendSource.contains(QStringLiteral("inlineHtmlImageRenderingEnabled")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string editorSurfaceHtml")));
}
