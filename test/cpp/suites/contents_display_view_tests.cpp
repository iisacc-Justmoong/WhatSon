#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QStringView>

void WhatSonCppRegressionTests::contentsDisplayView_invalidatesGutterGeometryImmediatelyAcrossRapidNoteSwitches()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));
    const QString geometryControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryController.qml"));
    const QString geometrySnapshotModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometrySnapshotModel.qml"));
    const QString inputOrchestrationModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputOrchestrationModel.qml"));
    const QString presentationOrchestrationModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayPresentationOrchestrationModel.qml"));
    const QString geometryStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryState.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(!geometryControllerSource.isEmpty());
    QVERIFY(!geometrySnapshotModelSource.isEmpty());
    QVERIFY(!inputOrchestrationModelSource.isEmpty());
    QVERIFY(!presentationOrchestrationModelSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("property alias minimapLineGroupsNoteId: geometryState.minimapLineGroupsNoteId")));
    QVERIFY(geometryStateSource.contains(QStringLiteral("property string minimapLineGroupsNoteId: \"\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function activeLineGeometryNoteId()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasPendingNoteEntryGutterRefresh(noteId)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function resetNoteEntryLineGeometryState()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId === contentsView.activeLineGeometryNoteId()")));
    QVERIFY(!geometryControllerSource.contains(QStringLiteral("currentNoteId === controller.contentsView.minimapLineGroupsNoteId")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.contentsView.buildStructuredMinimapLineGroupsForRange(")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.contentsView.buildMinimapLineGroupsForRange(")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.contentsView.minimapLineGroupsNoteId = currentNoteId;")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.contentsView.minimapLineGroupsNoteId = \"\";")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("eventPump.contentsView.scheduleStructuredDocumentOpenLayoutRefresh(\"rendered-blocks\")")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function scheduleStructuredDocumentOpenLayoutRefresh(reason)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayPresentationOrchestrationModel {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool structuredBlockBackgroundRefreshEnabled: contentsView.showStructuredDocumentFlow")));
    QVERIFY(displayViewSource.contains(QStringLiteral("|| contentsView.resourceBlocksRenderedInlineByHtmlProjection")));
    QVERIFY(displayViewSource.contains(QStringLiteral("backgroundRefreshEnabled: contentsView.structuredBlockBackgroundRefreshEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("!contentsView.selectedNoteBodyLoading")));
    QVERIFY(presentationOrchestrationModelSource.contains(
        QStringLiteral("model.contentsView.selectedNoteBodyNoteId === model.contentsView.selectedNoteId")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.scheduleStructuredDocumentOpenLayoutRefresh(\"selected-body-text\")")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.scheduleStructuredDocumentOpenLayoutRefresh(\"selected-body-resolved\")")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.scheduleStructuredDocumentOpenLayoutRefresh(\"selected-body-loading\")")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("eventPump.contentsView.scheduleViewportGutterRefresh();")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.refreshCoordinator.scheduleNoteEntryGutterRefresh(")));
    QVERIFY(presentationOrchestrationModelSource.contains(QStringLiteral("refreshPlan.gutterPassCount")));
    QVERIFY(presentationOrchestrationModelSource.contains(QStringLiteral("model.contentsView.scheduleGutterRefresh(")));
    QVERIFY(!geometrySnapshotModelSource.contains(QStringLiteral("buildNextMinimapSnapshotPlan(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayInputOrchestrationModel {")));
    QVERIFY(inputOrchestrationModelSource.contains(QStringLiteral("model.contextMenuCoordinator.openSelectionContextMenuPlan(")));
    QVERIFY(inputOrchestrationModelSource.contains(QStringLiteral("model.contextMenuCoordinator.inlineStyleTagForEvent(")));
    QVERIFY(inputOrchestrationModelSource.contains(QStringLiteral("model.contextMenuCoordinator.primeStructuredSelectionSnapshotPlan(")));
    QVERIFY(inputOrchestrationModelSource.contains(QStringLiteral("model.contextMenuCoordinator.structuredSelectionValid()")));
    QVERIFY(geometrySnapshotModelSource.contains(QStringLiteral("model.gutterCoordinator.buildVisiblePlainGutterLineEntries(")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("\"structured-layout-cache\"")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("\"editor-text-synchronized\"")));
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

void WhatSonCppRegressionTests::contentsDisplayView_doesNotInjectCurrentLineGutterDot()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString gutterLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsGutterLayer.qml"));
    const QString gutterMarkerBridgeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/bridge/ContentsGutterMarkerBridge.cpp"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!gutterLayerSource.isEmpty());
    QVERIFY(!gutterMarkerBridgeSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property var effectiveGutterMarkers")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("showCurrentLineMarker")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("gutterMarkerCurrentColor")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("currentCursorGutterLineHeight")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("currentCursorGutterLineY")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("\"type\": \"current\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("normalizedType === \"current\"")));
    QVERIFY(!gutterMarkerBridgeSource.contains(QStringLiteral("QStringLiteral(\"current\")")));
    QVERIFY(gutterLayerSource.contains(QStringLiteral("currentCursorLineNumber: active line used for highlight color and font weight"))
            || gutterLayerSource.contains(QStringLiteral("currentCursorLineNumber")));
}

void WhatSonCppRegressionTests::contentsDisplayView_reservesLargeBottomAccessibilityMargin()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString surfaceHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!surfaceHostSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property int editorBottomInset: Math.max(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("Math.round(contentsView.editorSurfaceHeight * 0.5)")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("+ contentsView.editorBottomInset)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("insetVertical: contentsView.showPrintEditorLayout ? 0 : contentsView.editorBottomInset")));
}

void WhatSonCppRegressionTests::contentsDisplayView_usesSelectedNoteSnapshotWhileSessionBindingCatchesUp()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString mountStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMountState.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!mountStateSource.isEmpty());
    QVERIFY(displayViewSource.contains(
        QStringLiteral("ContentsDisplayDocumentSourceResolver")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string documentPresentationSourceText: documentSourceResolver.documentPresentationSourceText")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property var documentSourcePlan: documentSourceResolver.documentSourcePlan")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("documentSourceResolver.resolveDocumentSourcePlan()")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("documentSourceResolver.resolvedDocumentPresentationSourceText()")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("editorBoundNoteDirectoryPath: contentsView.editorBoundNoteDirectoryPath === undefined || contentsView.editorBoundNoteDirectoryPath === null ? \"\" : String(contentsView.editorBoundNoteDirectoryPath)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? \"\" : String(contentsView.selectedNoteBodyNoteId)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? \"\" : String(contentsView.selectedNoteBodyText)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteDirectoryPath: contentsView.selectedNoteDirectoryPath === undefined || contentsView.selectedNoteDirectoryPath === null ? \"\" : String(contentsView.selectedNoteDirectoryPath)")));
    QVERIFY(mountStateSource.contains(
        QStringLiteral("return boundDirectoryPath === state.selectedNoteDirectoryPath;")));
}

void WhatSonCppRegressionTests::contentsDisplayView_focusesAlreadySelectedNoteOnMobileMount()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    const qsizetype completedIndex = displayViewSource.indexOf(QStringLiteral("Component.onCompleted: {"));
    const qsizetype focusOptionIndex = displayViewSource.indexOf(
        QStringLiteral("\"focusEditor\": contentsView.mobileHost && contentsView.hasSelectedNote"),
        completedIndex);

    QVERIFY(completedIndex >= 0);
    QVERIFY(focusOptionIndex > completedIndex);
    QVERIFY(displayViewSource.contains(QStringLiteral("\"resetSnapshot\": true")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"scheduleReconcile\": true")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"fallbackRefresh\": true")));
}

void WhatSonCppRegressionTests::contentsDisplayView_doesNotForceBlurFlushDuringNativeComposition()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString selectionMountControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionMountController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!selectionMountControllerSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("function nativeEditorCompositionActive()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function flushEditorStateAfterInputSettles(scheduledNoteId)")));
    QVERIFY(selectionMountControllerSource.contains(QStringLiteral("if (controller.contentsView.nativeEditorCompositionActive())\n            return;")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("contentsView.flushEditorStateAfterInputSettles(blurredNoteId);")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("retryCount < 6")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("flushEditorStateAfterInputSettles(retryCount + 1")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("flushEditorStateAfterInputSettles(0, blurredNoteId)")));
}

void WhatSonCppRegressionTests::qmlContextMenus_treatRightClickAndLongPressAsSymmetricPointerTriggers()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString inputCommandSurfaceSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml"));
    const QString inputOrchestrationModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputOrchestrationModel.qml"));
    const QString listBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"));
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!inputCommandSurfaceSource.isEmpty());
    QVERIFY(!inputOrchestrationModelSource.isEmpty());
    QVERIFY(!listBarSource.isEmpty());
    QVERIFY(!sidebarSource.isEmpty());

    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentContextMenuSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function editorContextMenuPointerTriggerAccepted(triggerKind)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function editorSelectionContextMenuSnapshotValid()")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function ensureEditorSelectionContextMenuSnapshot()")));
    QVERIFY(inputOrchestrationModelSource.contains(
        QStringLiteral("if (!model.ensureEditorSelectionContextMenuSnapshot())")));
    QVERIFY(inputOrchestrationModelSource.contains(
        QStringLiteral("return model.contextMenuCoordinator.structuredSelectionValid();")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("MouseArea {\n        id: editorRightClickContextMenuMouseArea")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("acceptedButtons: Qt.RightButton")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("preventStealing: true")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("commandSurface.contentsView.primeEditorSelectionContextMenuSnapshot();")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("commandSurface.contentsView.requestEditorSelectionContextMenuFromPointer(")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("readonly property bool nativeTouchSelectionGesturesPreferred: Qt.platform.os === \"ios\"")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("&& !commandSurface.nativeTouchSelectionGesturesPreferred")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("commandSurface.contentsView.requestEditorSelectionContextMenuFromPointer(")));
    QVERIFY(inputCommandSurfaceSource.contains(QStringLiteral("\"longPress\"")));

    QVERIFY(listBarSource.contains(
        QStringLiteral("function noteContextMenuPointerTriggerAccepted(triggerKind)")));
    QVERIFY(listBarSource.contains(
        QStringLiteral("function openNoteContextMenuFromPointer(delegateItem, localX, localY, triggerKind)")));
    QVERIFY(listBarSource.contains(
        QStringLiteral("listBarLayout.openNoteContextMenuFromPointer(noteItemDelegate, mouse.x, mouse.y, \"longPress\");")));
    QVERIFY(listBarSource.contains(
        QStringLiteral("listBarLayout.openNoteContextMenuFromPointer(noteItemDelegate, eventPoint.position.x, eventPoint.position.y, \"rightClick\");")));

    QVERIFY(sidebarSource.contains(
        QStringLiteral("function hierarchyContextMenuPointerTriggerAccepted(triggerKind)")));
    QVERIFY(sidebarSource.contains(
        QStringLiteral("function openHierarchyFolderContextMenuFromPointer(x, y, referenceItem, triggerKind)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"rightClick\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"longPress\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("onLongPressed: {")));
    QVERIFY(sidebarSource.contains(QStringLiteral("property string hierarchyViewOptionsTriggerQueuedAction: \"\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"eventName\": \"hierarchy.expandAll\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"eventName\": \"hierarchy.collapseAll\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"onTriggered\": function () {\n                sidebarHierarchyView.handleHierarchyViewOptionsTrigger(0, \"hierarchy.expandAll\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("\"onTriggered\": function () {\n                sidebarHierarchyView.handleHierarchyViewOptionsTrigger(1, \"hierarchy.collapseAll\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyViewOptionsActionName(index, eventName)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function requestHierarchyViewOptionsAction(index, eventName)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("if (sidebarHierarchyView.hierarchyViewOptionsTriggerQueuedAction === action)\n            return true;")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestExpandAllHierarchyItems();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestCollapseAllHierarchyItems();")));
}

void WhatSonCppRegressionTests::listBarLayout_rendersResolvedNoteListModelByIndex()
{
    const QString listBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"));

    QVERIFY(!listBarSource.isEmpty());
    QVERIFY(listBarSource.contains(QStringLiteral("readonly property var resolvedNoteListModel: listBarLayout.noteListModel")));
    QVERIFY(listBarSource.contains(QStringLiteral("model: listBarLayout.resolvedNoteListModel")));
    QVERIFY(listBarSource.contains(QStringLiteral("required property int index")));
    QVERIFY(listBarSource.contains(QStringLiteral("required property string noteId")));
    QVERIFY(listBarSource.contains(QStringLiteral("required property string primaryText")));
    QVERIFY(listBarSource.contains(QStringLiteral("required property string displayDate")));
    QVERIFY(listBarSource.contains(QStringLiteral("required property var folders")));
    QVERIFY(listBarSource.contains(QStringLiteral("required property var tags")));
    QVERIFY(!listBarSource.contains(QStringLiteral("model: listBarLayout.displayedNoteListEntries")));
    QVERIFY(!listBarSource.contains(QStringLiteral("required property var modelData")));
    QVERIFY(!listBarSource.contains(QStringLiteral("NoteListModelContractBridge {")));
    QVERIFY(!listBarSource.contains(QStringLiteral("noteListContractBridge")));
    QVERIFY(!listBarSource.contains(QStringLiteral("function readDisplayedNoteListEntriesFromModel")));
    QVERIFY(!listBarSource.contains(QStringLiteral("syncDisplayedNoteListEntries")));
}

void WhatSonCppRegressionTests::contentsDisplayView_routesStructuredMutationsThroughEditorSessionAuthority()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString mutationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMutationController.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorTypingController.qml"));
    const QString selectionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorSelectionController.qml"));
    const QString editOperationCoordinatorHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEditOperationCoordinator.hpp"));
    const QString editOperationCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEditOperationCoordinator.cpp"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!mutationControllerSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());
    QVERIFY(!selectionControllerSource.isEmpty());
    QVERIFY(!editOperationCoordinatorHeader.isEmpty());
    QVERIFY(!editOperationCoordinatorSource.isEmpty());
    QVERIFY(!mutationControllerSource.contains(QStringLiteral("documentSourceMutationPlan(")));
    QVERIFY(!mutationControllerSource.contains(QStringLiteral("applyStructuredSourceText")));
    QVERIFY(!mutationControllerSource.contains(QStringLiteral("applyEditorText")));
    QVERIFY(!editOperationCoordinatorHeader.contains(QStringLiteral("documentSourceMutationPlan")));
    QVERIFY(!editOperationCoordinatorSource.contains(QStringLiteral("documentSourceMutationPlan")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("controller.commitRawEditorTextMutation(normalizedNextSourceText)")));
    QVERIFY(!mutationControllerSource.contains(
        QStringLiteral("controller.contentsView.editorText = normalizedNextSourceText;")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("controller.commitRawEditorTextMutation(nextSourceText)")));
    QVERIFY(selectionControllerSource.contains(QStringLiteral("controller.commitRawEditorTextMutation(nextText)")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("controller.view.editorText = nextSourceText;")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("controller.view.editorText = normalizedNextSourceText;")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("controller.view.editorText = nextText;")));
    QVERIFY(!mutationControllerSource.contains(
        QStringLiteral("contentsView.structuredFlowSourceText = normalizedNextSourceText;")));
    QVERIFY(!mutationControllerSource.contains(
        QStringLiteral("controller.editorSession.markLocalEditorAuthority")));
    QVERIFY(!mutationControllerSource.contains(
        QStringLiteral("controller.editorSession.scheduleEditorPersistence")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("controller.editorSession.markLocalEditorAuthority")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("controller.editorSession.scheduleEditorPersistence")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("controller.editorSession.markLocalEditorAuthority")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("controller.editorSession.scheduleEditorPersistence")));
}

void WhatSonCppRegressionTests::contentsDisplayView_refreshesMinimapFromResolvedPresentationSource()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(
        QStringLiteral("onDocumentPresentationSourceTextChanged: {")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.refreshLiveLogicalLineMetrics();")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("minimapSnapshotForceFullRefresh")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.scheduleMinimapSnapshotRefresh(true);")));
}

void WhatSonCppRegressionTests::contentsDisplayView_scalesMinimapRowsFromDocumentGeometry()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString viewportModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayViewportModel.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!viewportModelSource.isEmpty());
    QVERIFY(displayViewSource.contains(
        QStringLiteral("EditorDisplayModel.ContentsDisplayViewportModel {")));
    QVERIFY(viewportModelSource.contains(
        QStringLiteral("viewportCoordinator.minimapTrackHeightForContentHeight(")));
    QVERIFY(viewportModelSource.contains(
        QStringLiteral("viewportCoordinator.minimapTrackYForContentY(")));
    QVERIFY(viewportModelSource.contains(
        QStringLiteral("structuredDocumentFlow.normalizedResolvedInteractiveBlockIndex()")));
    QVERIFY(viewportModelSource.contains(
        QStringLiteral("Math.ceil(model.minimapContentHeight() / safeEditorLineHeight)")));
    QVERIFY(!viewportModelSource.contains(
        QStringLiteral("return rows.length * contentsView.minimapVisualRowPaintHeight(rows[0]) + Math.max(0, rows.length - 1) * contentsView.minimapRowGap;")));
    QVERIFY(!viewportModelSource.contains(
        QStringLiteral("return visualIndex * (contentsView.minimapRowGap + contentsView.minimapVisualRowPaintHeight(rowSpec));")));
}

void WhatSonCppRegressionTests::contentsDisplayView_refreshesMinimapWhenStructuredLayoutCacheChanges()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("function onCachedLogicalLineEntriesChanged() {")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("const metricsChanged = eventPump.contentsView.refreshLiveLogicalLineMetrics();")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("const geometryChanged = eventPump.contentsView.consumeStructuredGutterGeometryChange();")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("eventPump.contentsView.scheduleMinimapSnapshotRefresh(true);")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("eventPump.contentsView.scheduleGutterRefresh(")));
}

void WhatSonCppRegressionTests::contentsDisplayView_rebuildsMinimapRowsFromCurrentDocumentState()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString geometrySnapshotModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometrySnapshotModel.qml"));
    const QString minimapCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMinimapCoordinator.cpp"));
    const QString geometryStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryState.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!geometrySnapshotModelSource.isEmpty());
    QVERIFY(!minimapCoordinatorSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayGeometrySnapshotModel {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("property alias minimapSnapshotEntries: geometryState.minimapSnapshotEntries")));
    QVERIFY(!geometryStateSource.contains(QStringLiteral("property var minimapSnapshotEntries: []")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("property string minimapSnapshotSourceText: \"\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasStructuredLogicalLineGeometry()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function effectiveStructuredMinimapEntries()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasStructuredMinimapEntries()")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function currentMinimapSnapshotEntries()")));
    QVERIFY(geometrySnapshotModelSource.contains(QStringLiteral("minimapCoordinator.buildStructuredMinimapSnapshotEntries(")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function minimapSnapshotEntriesEqual(previousEntries, nextEntries)")));
    QVERIFY(!geometrySnapshotModelSource.contains(QStringLiteral("model.contentsView.minimapSnapshotEntries,")));
    QVERIFY(!geometrySnapshotModelSource.contains(QStringLiteral("buildNextMinimapSnapshotPlan(")));
    QVERIFY(!geometrySnapshotModelSource.contains(QStringLiteral("spliceLineGroups(")));
    QVERIFY(geometrySnapshotModelSource.contains(QStringLiteral("model.contentsView.documentPresentationSourceText !== undefined")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredHostGeometryActive: contentsView.structuredHostGeometryActive")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("return contentsView.normalizedSnapshotEntries(contentsView.effectiveStructuredLogicalLineEntries());")));
    QVERIFY(minimapCoordinatorSource.contains(QStringLiteral("buildStructuredMinimapSnapshotEntries")));
    QVERIFY(!minimapCoordinatorSource.contains(QStringLiteral("buildNextMinimapSnapshotPlan")));
    QVERIFY(minimapCoordinatorSource.contains(QStringLiteral("QStringLiteral(\"block|%1|%2|%3|%4|%5\")")));
}

void WhatSonCppRegressionTests::contentsDisplayView_keepsSingleResolverBindingPerDocumentSourceProperty()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());

    const QString resolverStartMarker = QStringLiteral("ContentsDisplayDocumentSourceResolver {");
    const QString resolverEndMarker = QStringLiteral("    ContentsDisplayNoteBodyMountCoordinator {");
    const int resolverStart = displayViewSource.indexOf(resolverStartMarker);
    QVERIFY(resolverStart >= 0);
    const int resolverEnd = displayViewSource.indexOf(resolverEndMarker, resolverStart);
    QVERIFY(resolverEnd > resolverStart);

    const QString resolverBlock = displayViewSource.mid(resolverStart, resolverEnd - resolverStart);

    const auto countOccurrences = [&resolverBlock](QStringView token) {
        int count = 0;
        int searchFrom = 0;
        const QString needle = token.toString();
        while (true) {
            const int foundIndex = resolverBlock.indexOf(needle, searchFrom);
            if (foundIndex < 0)
                return count;
            ++count;
            searchFrom = foundIndex + needle.size();
        }
    };

    QCOMPARE(countOccurrences(u"editorBoundNoteId:"), 1);
    QCOMPARE(countOccurrences(u"editorBoundNoteDirectoryPath:"), 1);
    QCOMPARE(countOccurrences(u"editorText:"), 1);
    QCOMPARE(countOccurrences(u"pendingBodySave:"), 1);
    QCOMPARE(countOccurrences(u"selectedNoteBodyNoteId:"), 1);
    QCOMPARE(countOccurrences(u"selectedNoteBodyResolved:"), 1);
    QCOMPARE(countOccurrences(u"selectedNoteBodyText:"), 1);
    QCOMPARE(countOccurrences(u"selectedNoteDirectoryPath:"), 1);
    QCOMPARE(countOccurrences(u"selectedNoteId:"), 1);
    QCOMPARE(countOccurrences(u"structuredFlowSourceText:"), 1);
}

void WhatSonCppRegressionTests::contentsDisplayView_emitsEditorCreationTraceAcrossHostTransitions()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString surfaceHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml"));
    const QString presentationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayPresentationController.qml"));
    const QString presentationOrchestrationModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayPresentationOrchestrationModel.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!surfaceHostSource.isEmpty());
    QVERIFY(!presentationControllerSource.isEmpty());
    QVERIFY(!presentationOrchestrationModelSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayTraceFormatter")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("\" activeSurface=\" + controller.contentsView.activeSurfaceKind")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("traceFormatter.describeEditorSurfaceObject(controller.structuredDocumentFlow)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayPresentationOrchestrationModel {")));
    QVERIFY(presentationOrchestrationModelSource.contains(QStringLiteral("model.presentationViewModel.logEditorCreationState(reason)")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("function logEditorCreationState(reason)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.logEditorCreationState(\"componentCompleted\");")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplaySelectionBridge\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayNoteBodyMountCoordinator\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplaySessionCoordinator\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("id: documentSourceResolver")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayEditorSession\"")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("objectName: \"contentsDisplayStructuredDocumentFlow\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayInlineFormatEditor\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayInlineEditorLoader\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onSelectedNoteBodyNoteIdChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onEditorBoundNoteIdChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onEditorSessionBoundToSelectedNoteChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onStructuredDocumentFlowRequestedChanged:")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("onLegacyInlineEditorRequestedChanged:")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("onLegacyInlineEditorActiveChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onNoteDocumentMountPendingChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onNoteDocumentMountedChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onNoteDocumentMountFailureReasonChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"selectionBridgeCreated\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"noteBodyMountCoordinatorCreated\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("\"sessionCoordinatorCreated\"")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("documentSourceResolver={")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"editorSessionCreated\"")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("\"structuredDocumentFlowVisibleChanged\"")));
    QVERIFY(!eventPumpSource.contains(QStringLiteral("\"contentEditorLoaderLoaded\"")));
    QVERIFY(!eventPumpSource.contains(QStringLiteral("\"contentEditorLoaderStatusChanged\"")));
}

void WhatSonCppRegressionTests::contentsDisplayView_tracesNoteSelectionPlanExecution()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString selectionMountControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionMountController.qml"));
    const QString selectionOrchestrationModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionOrchestrationModel.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!selectionMountControllerSource.isEmpty());
    QVERIFY(!selectionOrchestrationModelSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplaySelectionOrchestrationModel {")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("traceFormatter.describeSelectionSyncOptions(normalizedOptions)")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("traceFormatter.describeSelectionPlan(mountPlan)")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("traceFormatter.describeSelectionPlan(pollPlan)")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("traceFormatter.describeSelectionPlan(reconcilePlan)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("contentsView.describeSelectionPlan(")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.scheduleSelectionModelSync\"")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.pollPlan\"")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.pollReconcileRequested\"")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.pollSkipped\"")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.pollSnapshotRefresh\"")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.reconcilePlan\"")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("\"selectionFlow.reconcileRequested\"")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("\"selectionFlow.selectionSyncPlan\"")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("\"selectionFlow.selectionSyncResult\"")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("\"selectionFlow.mountPlan\"")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("\"selectionFlow.mountResult\"")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("function onSelectionSyncFlushRequested(plan)")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("scheduledFollowUpMount")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("traceFormatter.describeSelectionPlan(mountPlan)")));
}

void WhatSonCppRegressionTests::contentsDisplayView_surfacesMountFailurePlaceholderWithoutChrome()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString geometryStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryState.qml"));
    const QString presentationStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayPresentationState.qml"));
    const QString resourceUiStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayResourceUiState.qml"));
    const QString mountStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMountState.qml"));
    const QString inputStateSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputState.qml"));
    const QString selectionOrchestrationModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionOrchestrationModel.qml"));
    const QString selectionMountControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionMountController.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));
    const QString auxiliaryHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayAuxiliaryRailHost.qml"));
    const QString surfaceHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml"));
    const QString overlayHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayOverlayHost.qml"));
    const QString gutterHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayGutterHost.qml"));
    const QString minimapHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayMinimapRailHost.qml"));
    const QString exceptionOverlaySource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayExceptionOverlay.qml"));
    const QString resourceImportConflictAlertSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayResourceImportConflictAlert.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!selectionOrchestrationModelSource.isEmpty());
    QVERIFY(!selectionMountControllerSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(!auxiliaryHostSource.isEmpty());
    QVERIFY(!surfaceHostSource.isEmpty());
    QVERIFY(!overlayHostSource.isEmpty());
    QVERIFY(!gutterHostSource.isEmpty());
    QVERIFY(!minimapHostSource.isEmpty());
    QVERIFY(!exceptionOverlaySource.isEmpty());
    QVERIFY(!resourceImportConflictAlertSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayNoteBodyMountCoordinator")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplaySurfacePolicy")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayGeometryState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayPresentationState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayResourceUiState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayMountState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayInputState")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplaySelectionOrchestrationModel {")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool selectedNoteBodyResolved: mountState.selectedNoteBodyResolved")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool structuredDocumentFlowRequested: inputState.structuredDocumentFlowRequested")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("legacyInlineEditorRequested")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("legacyInlineEditorActive")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteSnapshotRefreshEnabled: contentsView.visible && contentsView.hasSelectedNote && !contentsView.selectedNoteBodyLoading && (contentsView.editorSessionBoundToSelectedNote || contentsView.selectedNoteBodyResolved)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMountDecisionClean: mountState.noteDocumentMountDecisionClean")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMountPending: mountState.noteDocumentMountPending")));
    QVERIFY(mountStateSource.contains(
        QStringLiteral("&& !state.noteDocumentMountDecisionClean")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property var documentSourcePlan: documentSourceResolver.documentSourcePlan")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("inlineDocumentSurfaceRequested: surfacePolicy.inlineDocumentSurfaceRequested")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("editorBoundNoteId: contentsView.editorBoundNoteId === undefined || contentsView.editorBoundNoteId === null ? \"\" : String(contentsView.editorBoundNoteId)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("editorSessionBoundToSelectedNote: contentsView.editorSessionBoundToSelectedNote")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("editorText: contentsView.editorText === undefined || contentsView.editorText === null ? \"\" : String(contentsView.editorText)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? \"\" : String(contentsView.selectedNoteBodyNoteId)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? \"\" : String(contentsView.selectedNoteBodyText)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("editorBoundNoteId: contentsView.documentSourcePlan.value(\"editorBoundNoteId\", \"\")")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("selectedNoteBodyResolved: contentsView.documentSourcePlan.value(\"resolvedSourceReady\", false)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("selectedNoteBodyText: contentsView.documentSourcePlan.value(\"preferEditorSessionSource\", false)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("contentEditorLoader")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("structuredDocumentSurfaceRequested: contentsView.structuredDocumentFlowRequested")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("structuredDocumentSurfaceReady: contentsView.structuredDocumentFlowRequested")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("&& structuredDocumentFlow.visible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentParseMounted: mountState.noteDocumentParseMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentSourceMounted: mountState.noteDocumentSourceMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMounted: mountState.noteDocumentMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMountFailureVisible: mountState.noteDocumentMountFailureVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentMountFailureReason: mountState.noteDocumentMountFailureReason")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentMountFailureMessage: mountState.noteDocumentMountFailureMessage")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentSurfaceVisible: noteBodyMountCoordinator.surfaceVisible")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentSurfaceInteractive: noteBodyMountCoordinator.surfaceInteractive")));
    QVERIFY(surfaceHostSource.contains(
        QStringLiteral("enabled: contentsView.hasSelectedNote")));
    QVERIFY(surfaceHostSource.contains(
        QStringLiteral("&& !contentsView.noteDocumentExceptionVisible")));
    QVERIFY(!surfaceHostSource.contains(
        QStringLiteral("enabled: contentsView.noteDocumentParseMounted")));
    QVERIFY(!surfaceHostSource.contains(
        QStringLiteral("enabled: contentsView.noteDocumentSurfaceInteractive")));
    QVERIFY(!surfaceHostSource.contains(
        QStringLiteral("enabled: !contentsView.selectedNoteBodyLoading")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionReason: mountState.noteDocumentExceptionReason")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionTitle: mountState.noteDocumentExceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionMessage: mountState.noteDocumentExceptionMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentExceptionVisible: mountState.noteDocumentExceptionVisible")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentCommandSurfaceEnabled: contentsView.noteDocumentParseMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("noteDocumentParseMounted: contentsView.noteDocumentParseMounted")));
    QVERIFY(mountStateSource.contains(QStringLiteral("property var editorSession: null")));
    QVERIFY(mountStateSource.contains(QStringLiteral("property var selectionBridge: null")));
    QVERIFY(mountStateSource.contains(QStringLiteral("property var noteBodyMountCoordinator: null")));
    QVERIFY(inputStateSource.contains(QStringLiteral("readonly property bool nativeTextInputPriority: !!(surfacePolicy && surfacePolicy.nativeInputPriority)")));
    QVERIFY(inputStateSource.contains(QStringLiteral("readonly property bool contextMenuLongPressEnabled: !!(editorInputPolicyAdapter && editorInputPolicyAdapter.contextMenuLongPressEnabled)")));
    QVERIFY(geometryStateSource.contains(QStringLiteral("property var minimapLineGroups: []")));
    QVERIFY(geometryStateSource.contains(QStringLiteral("property var visibleGutterLineEntries: [")));
    QVERIFY(presentationStateSource.contains(QStringLiteral("property string renderedEditorHtml: \"\"")));
    QVERIFY(resourceUiStateSource.contains(QStringLiteral("property bool resourceDropActive: false")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("const normalizedOptions = options && typeof options === \"object\" ? options : ({});")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("selectionSyncCoordinator.scheduleSelectionSync(normalizedOptions);")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("noteBodyMountCoordinator.scheduleMount(normalizedOptions);")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("target: eventPump.noteBodyMountCoordinator")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function executeSelectionDeliveryPlan(plan, fallbackKey)")));
    QVERIFY(selectionOrchestrationModelSource.contains(
        QStringLiteral("return model.selectionMountViewModel.executeSelectionDeliveryPlan(plan, fallbackKey);")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("function onSelectionSyncFlushRequested(plan)")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("function onMountFlushRequested(plan)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function normalizedStructuredSelectionContextMenuSnapshot(snapshot)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function structuredContextMenuSelectionValid()")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function structuredContextMenuInlineStyleTag(eventName)")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("normalizedPlan.attemptSelectionSync || normalizedPlan.attemptEditorSessionMount")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("mountPlan.attemptSnapshotRefresh")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("noteBodyMountCoordinator.handleSnapshotRefreshFinished(")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("const refreshedNoteId = String(mountPlan.selectedNoteId || \"\").trim();")));
    QVERIFY(eventPumpSource.contains(
        QStringLiteral("if (!eventPump.contentsView.selectedNoteBodyLoading")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("normalizedPlan.attemptSelectionSync || normalizedPlan.attemptEditorSessionMount")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("normalizedPlan[fallbackKey]")));
    QVERIFY(selectionMountControllerSource.contains(
        QStringLiteral("normalizedPlan[fallbackKey]")));
    QVERIFY(auxiliaryHostSource.contains(
        QStringLiteral("visible: auxiliaryHost.contentsView.hasSelectedNote")));
    QVERIFY(auxiliaryHostSource.contains(
        QStringLiteral("&& !auxiliaryHost.contentsView.noteDocumentExceptionVisible")));
    QVERIFY(!auxiliaryHostSource.contains(
        QStringLiteral("visible: contentsView.hasSelectedNote && contentsView.noteDocumentParseMounted")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("id: editorActivationSurface")));
    const qsizetype activationSurfaceIndex = auxiliaryHostSource.indexOf(
        QStringLiteral("id: editorActivationSurface"));
    const qsizetype editorRowIndex = auxiliaryHostSource.indexOf(QStringLiteral("RowLayout {"));
    const qsizetype gutterHostIndex = auxiliaryHostSource.indexOf(
        QStringLiteral("ContentsDisplayGutterHost {"),
        editorRowIndex);
    const qsizetype activationSurfaceZIndex = auxiliaryHostSource.indexOf(
        QStringLiteral("z: 0"),
        activationSurfaceIndex);
    const qsizetype editorRowZIndex = auxiliaryHostSource.indexOf(
        QStringLiteral("z: 1"),
        editorRowIndex);
    QVERIFY(activationSurfaceIndex >= 0);
    QVERIFY(editorRowIndex > activationSurfaceIndex);
    QVERIFY(gutterHostIndex > editorRowIndex);
    QVERIFY(activationSurfaceZIndex > activationSurfaceIndex);
    QVERIFY(activationSurfaceZIndex < editorRowIndex);
    QVERIFY(editorRowZIndex > editorRowIndex);
    QVERIFY(editorRowZIndex < gutterHostIndex);
    QVERIFY(!auxiliaryHostSource.contains(QStringLiteral("z: 10")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("editorWholeSurfaceTrailingMarginTapHandler")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("requestTerminalBodyClickFromActivationPoint(localX, localY)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayAuxiliaryRailHost")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayOverlayHost")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("ContentsDisplayGutterHost")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("ContentsDisplayMinimapRailHost")));
    QVERIFY(overlayHostSource.contains(QStringLiteral("ContentsDisplayExceptionOverlay")));
    QVERIFY(!surfaceHostSource.contains(QStringLiteral("ContentsDisplayMountLoadingOverlay")));
    QVERIFY(!surfaceHostSource.contains(QStringLiteral("Loading note")));
    QVERIFY(overlayHostSource.contains(QStringLiteral("ContentsDisplayResourceImportConflictAlert")));
    QVERIFY(resourceImportConflictAlertSource.contains(QStringLiteral("LV.Alert")));
    QVERIFY(resourceImportConflictAlertSource.contains(QStringLiteral("resourceImportController.executePendingResourceImportWithPolicy")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("ContentsGutterLayer {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("ContentsMinimapLayer {")));
    QVERIFY(gutterHostSource.contains(
        QStringLiteral("visible: contentsView.showEditorGutter && contentsView.noteDocumentParseMounted")));
    QVERIFY(exceptionOverlaySource.contains(
        QStringLiteral("visible: contentsView.noteDocumentExceptionVisible")));
    QVERIFY(exceptionOverlaySource.contains(
        QStringLiteral("text: exceptionOverlay.contentsView.noteDocumentExceptionTitle")));
    QVERIFY(exceptionOverlaySource.contains(
        QStringLiteral("text: exceptionOverlay.contentsView.noteDocumentExceptionMessage")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Loading note...")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionTitle: mountState.noteDocumentExceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionMessage: mountState.noteDocumentExceptionMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionReason: mountState.noteDocumentExceptionReason")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionTitle: mountState.noteDocumentExceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionMessage: mountState.noteDocumentExceptionMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("onSelectedNoteBodyLoadingChanged:")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("if (contentsView.selectedNoteBodyLoading)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("if (contentsView.selectedNoteId.length === 0)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("onSelectedNoteBodyResolvedChanged:")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("&& !contentsView.selectedNoteBodyResolved)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function focusEditorForSelectedNoteId(noteId)")));
    QVERIFY(auxiliaryHostSource.contains(
        QStringLiteral("visible: contentsView.showMinimapRail && contentsView.noteDocumentParseMounted")));
}
