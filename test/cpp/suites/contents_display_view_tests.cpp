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

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(!geometryControllerSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("property string minimapLineGroupsNoteId: \"\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function activeLineGeometryNoteId()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasPendingNoteEntryGutterRefresh(noteId)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function resetNoteEntryLineGeometryState()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId === contentsView.activeLineGeometryNoteId()")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("currentNoteId === controller.contentsView.minimapLineGroupsNoteId")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.contentsView.minimapLineGroupsNoteId = currentNoteId;")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.contentsView.minimapLineGroupsNoteId = \"\";")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("eventPump.structuredDocumentFlow.scheduleLayoutCacheRefresh();")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("eventPump.contentsView.scheduleViewportGutterRefresh();")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("controller.refreshCoordinator.scheduleNoteEntryGutterRefresh(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("refreshPlan.gutterPassCount")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleGutterRefresh(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("minimapCoordinator.buildNextMinimapSnapshotPlan(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contextMenuCoordinator.openSelectionContextMenuPlan(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contextMenuCoordinator.inlineStyleTagForEvent(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contextMenuCoordinator.primeStructuredSelectionSnapshotPlan(")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contextMenuCoordinator.structuredSelectionValid()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("gutterCoordinator.buildVisiblePlainGutterLineEntries(")));
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
    QVERIFY(displayViewSource.contains(
        QStringLiteral("return editorSession.editorBoundNoteDirectoryPath === contentsView.selectedNoteDirectoryPath;")));
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
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.flushEditorStateAfterInputSettles(blurredNoteId);")));
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
    const QString listBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"));
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!inputCommandSurfaceSource.isEmpty());
    QVERIFY(!listBarSource.isEmpty());
    QVERIFY(!sidebarSource.isEmpty());

    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentContextMenuSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function editorContextMenuPointerTriggerAccepted(triggerKind)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("function requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind)")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("commandSurface.contentsView.requestEditorSelectionContextMenuFromPointer(lastPressX, lastPressY, \"rightClick\");")));
    QVERIFY(inputCommandSurfaceSource.contains(
        QStringLiteral("acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus")));
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
}

void WhatSonCppRegressionTests::contentsDisplayView_routesStructuredMutationsThroughEditorSessionAuthority()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString mutationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMutationController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!mutationControllerSource.isEmpty());
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("if (mutationPlan.applyStructuredSourceText || mutationPlan.applyEditorText) {")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("if (controller.contentsView.editorText !== normalizedNextSourceText)")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("controller.contentsView.editorText = normalizedNextSourceText;")));
    QVERIFY(!mutationControllerSource.contains(
        QStringLiteral("contentsView.structuredFlowSourceText = normalizedNextSourceText;")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("controller.editorSession.markLocalEditorAuthority")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("controller.editorSession.scheduleEditorPersistence")));
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
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.minimapSnapshotForceFullRefresh = true;")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("contentsView.scheduleMinimapSnapshotRefresh(true);")));
}

void WhatSonCppRegressionTests::contentsDisplayView_scalesMinimapRowsFromDocumentGeometry()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(
        QStringLiteral("viewportCoordinator.minimapTrackHeightForContentHeight(")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("viewportCoordinator.minimapTrackYForContentY(")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("Math.ceil(contentsView.minimapContentHeight() / safeEditorLineHeight)")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("return rows.length * contentsView.minimapVisualRowPaintHeight(rows[0]) + Math.max(0, rows.length - 1) * contentsView.minimapRowGap;")));
    QVERIFY(!displayViewSource.contains(
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

void WhatSonCppRegressionTests::contentsDisplayView_normalizesMinimapSnapshotsAgainstStructuredEntries()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("property var minimapSnapshotEntries: []")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("property string minimapSnapshotSourceText: \"\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasStructuredLogicalLineGeometry()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function currentMinimapSnapshotEntries()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function minimapSnapshotEntriesEqual(previousEntries, nextEntries)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapSnapshotEntries,")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.documentPresentationSourceText !== undefined")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredHostGeometryActive: contentsView.hasStructuredLogicalLineGeometry()")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("function snapshotTokenForLogicalLine(blockEntry, logicalLines, lineIndex)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"snapshotToken\": documentFlow.snapshotTokenForLogicalLine(blockEntry, logicalLines, index)")));
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
    const QString presentationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayPresentationController.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!presentationControllerSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayTraceFormatter")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("traceFormatter.loaderStatusName(controller.contentEditorLoader.status)")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("traceFormatter.describeEditorSurfaceObject(controller.contentEditorLoader.item)")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("function logEditorCreationState(reason)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.logEditorCreationState(\"componentCompleted\");")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplaySelectionBridge\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayNoteBodyMountCoordinator\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplaySessionCoordinator\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("id: documentSourceResolver")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayEditorSession\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayStructuredDocumentFlow\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayInlineFormatEditor\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("objectName: \"contentsDisplayInlineEditorLoader\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onSelectedNoteBodyNoteIdChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onEditorBoundNoteIdChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onEditorSessionBoundToSelectedNoteChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onStructuredDocumentFlowRequestedChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onLegacyInlineEditorRequestedChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onLegacyInlineEditorActiveChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onNoteDocumentMountPendingChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onNoteDocumentMountedChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onNoteDocumentMountFailureReasonChanged:")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"selectionBridgeCreated\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"noteBodyMountCoordinatorCreated\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("\"sessionCoordinatorCreated\"")));
    QVERIFY(presentationControllerSource.contains(QStringLiteral("documentSourceResolver={")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"editorSessionCreated\"")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("\"structuredDocumentFlowVisibleChanged\"")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("\"contentEditorLoaderLoaded\"")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("\"contentEditorLoaderStatusChanged\"")));
}

void WhatSonCppRegressionTests::contentsDisplayView_tracesNoteSelectionPlanExecution()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString selectionMountControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionMountController.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!selectionMountControllerSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
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
    const QString selectionMountControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplaySelectionMountController.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!selectionMountControllerSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
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
        QStringLiteral("readonly property var documentSourcePlan: documentSourceResolver.documentSourcePlan")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("inlineDocumentSurfaceRequested: contentsView.legacyInlineEditorRequested")));
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
    QVERIFY(displayViewSource.contains(
        QStringLiteral("inlineDocumentSurfaceReady: contentEditorLoader.status === Loader.Ready && contentEditorLoader.item !== null")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("structuredDocumentSurfaceRequested: contentsView.structuredDocumentFlowRequested")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("structuredDocumentSurfaceReady: contentsView.structuredDocumentFlowRequested && structuredDocumentFlow.visible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentParseMounted: noteBodyMountCoordinator.parseMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentSourceMounted: noteBodyMountCoordinator.sourceMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMounted: noteBodyMountCoordinator.noteMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentMountFailureVisible: noteBodyMountCoordinator.mountFailed")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentMountFailureReason: noteBodyMountCoordinator.mountFailureReason")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentMountFailureMessage: noteBodyMountCoordinator.mountFailureMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentSurfaceVisible: noteBodyMountCoordinator.surfaceVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionReason: noteBodyMountCoordinator.exceptionReason")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionTitle: noteBodyMountCoordinator.exceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionMessage: noteBodyMountCoordinator.exceptionMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentExceptionVisible: noteBodyMountCoordinator.exceptionVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentCommandSurfaceEnabled: noteBodyMountCoordinator.commandSurfaceEnabled")));
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
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.hasSelectedNote && contentsView.noteDocumentSurfaceVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.showEditorGutter && contentsView.noteDocumentParseMounted")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.noteDocumentMountPending")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.noteDocumentExceptionVisible")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("text: contentsView.noteDocumentExceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("text: contentsView.noteDocumentExceptionMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("text: \"Loading note...\"")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionTitle: noteBodyMountCoordinator.exceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionMessage: noteBodyMountCoordinator.exceptionMessage")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionReason: noteBodyMountCoordinator.exceptionReason")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionTitle: noteBodyMountCoordinator.exceptionTitle")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property string noteDocumentExceptionMessage: noteBodyMountCoordinator.exceptionMessage")));
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
    QVERIFY(displayViewSource.contains(
        QStringLiteral("visible: contentsView.showMinimapRail && contentsView.noteDocumentParseMounted")));
}
