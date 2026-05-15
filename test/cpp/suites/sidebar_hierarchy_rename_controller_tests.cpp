#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarHierarchyRenameController_preservesLiteralSlashFolderLabels()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(sidebarSource.contains(QStringLiteral("function decodedHierarchyPathSegments(rawPath)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function leafHierarchyItemLabel(rawLabel, rawPath)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("if (nextCharacter === \"\\\\\" || nextCharacter === \"/\")")));
    QVERIFY(sidebarSource.contains(QStringLiteral("renameController.leafHierarchyItemLabel(item.label, item.id)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("item && item.id !== undefined && item.id !== null ? item.id : \"\"")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("const segments = normalizedLabel.split(\"/\")")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_bindsInlineHelperDependenciesAtStartup()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(sidebarSource.contains(QStringLiteral("property var view: sidebarHierarchyView")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyController: sidebarHierarchyView.hierarchyController")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyInteractionBridge: sidebarHierarchyView.hierarchyInteractionBridge")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyRenameField: sidebarHierarchyView.hierarchyRenameFieldItem")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyDragDropBridge: sidebarHierarchyView.hierarchyDragDropBridge")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyTree: sidebarHierarchyView.hierarchyTreeItem")));
    QVERIFY(sidebarSource.contains(QStringLiteral("bookmarkCanvas: sidebarHierarchyView.bookmarkPaletteCanvasItem")));
    QVERIFY(sidebarSource.contains(QStringLiteral("itemLocator: noteDropController")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("property var view: null")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("const normalizedModifiers = controller.")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_bindsLvrsHierarchyDirectlyToSharedItemModel()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());

    QVERIFY(sidebarSource.contains(
        QStringLiteral("readonly property var hierarchyRenderModel: sidebarHierarchyView.hierarchyController ? sidebarHierarchyView.hierarchyController.hierarchyItemModel : []")));
    QVERIFY(sidebarSource.contains(QStringLiteral("model: sidebarHierarchyView.hierarchyRenderModel")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyReorderCommitModel()")));
    QVERIFY(sidebarSource.contains(QStringLiteral("return itemModel.items();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("applyHierarchyReorder(sidebarHierarchyView.hierarchyReorderCommitModel(), itemKey)")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("property var displayedHierarchyModel")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("property string displayedHierarchyModelSignature")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("sidebarHierarchyView.displayedHierarchyModel =")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_waitsForCreatedFolderRowBeforeInlineRename()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());

    const qsizetype createFunctionIndex = sidebarSource.indexOf(QStringLiteral("function requestCreateFolder(targetIndex, reason)"));
    QVERIFY(createFunctionIndex >= 0);
    const qsizetype baselineCaptureIndex = sidebarSource.indexOf(
        QStringLiteral("const createBaselineModel = renameController.normalizeHierarchyModel(sidebarHierarchyView.hierarchyController.hierarchyNodes);"),
        createFunctionIndex);
    const qsizetype createCallIndex = sidebarSource.indexOf(QStringLiteral("sidebarHierarchyView.hierarchyInteractionBridge.createFolder();"), createFunctionIndex);
    const qsizetype createdKeyCaptureIndex = sidebarSource.indexOf(
        QStringLiteral("const createdFolderKey = sidebarHierarchyView.insertedHierarchyModelItemKey(createBaselineModel, sidebarHierarchyView.standardHierarchyModel);"),
        createCallIndex);
    const qsizetype deferredRenameIndex = sidebarSource.indexOf(
        QStringLiteral("renameController.beginRenameHierarchyItemKeyWhenVisible(createdFolderKey, createdFolderIndex, 8);"),
        createdKeyCaptureIndex);
    QVERIFY(baselineCaptureIndex > createFunctionIndex);
    QVERIFY(createCallIndex > createFunctionIndex);
    QVERIFY(createdKeyCaptureIndex > createCallIndex);
    QVERIFY(deferredRenameIndex > createdKeyCaptureIndex);
    QVERIFY(!sidebarSource.contains(QStringLiteral("const createdFolderIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.selectedFolderIndex, -1);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function insertedHierarchyModelItemIndex(beforeModel, afterModel, fallbackIndex)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function insertedHierarchyModelItemKey(beforeModel, afterModel)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyModelIndexForKey(itemKey)")));

    const qsizetype waitFunctionIndex = sidebarSource.indexOf(QStringLiteral("function beginRenameHierarchyItemWhenVisible(renameIndex, remainingAttempts)"));
    QVERIFY(waitFunctionIndex >= 0);
    const qsizetype readyCheckIndex = sidebarSource.indexOf(QStringLiteral("renameController.renamePresentationReady(normalizedIndex)"), waitFunctionIndex);
    const qsizetype retryIndex = sidebarSource.indexOf(QStringLiteral("Qt.callLater(function () {\n                renameController.beginRenameHierarchyItemWhenVisible(normalizedIndex, nextAttempts);"), readyCheckIndex);
    const qsizetype waitFunctionEndIndex = sidebarSource.indexOf(QStringLiteral("function focusHierarchyRenameField(renameIndex)"), waitFunctionIndex);
    QVERIFY(waitFunctionEndIndex > waitFunctionIndex);
    const QString waitFunctionBlock = sidebarSource.mid(waitFunctionIndex, waitFunctionEndIndex - waitFunctionIndex);
    QVERIFY(readyCheckIndex > waitFunctionIndex);
    QVERIFY(retryIndex > readyCheckIndex);
    QVERIFY(waitFunctionBlock.contains(QStringLiteral("hostView.syncSelectedHierarchyItem(false);")));
    QVERIFY(!waitFunctionBlock.contains(QStringLiteral("hostView.syncSelectedHierarchyItem(true);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function beginRenameHierarchyItemKeyWhenVisible(itemKey, fallbackIndex, remainingAttempts)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("const resolvedIndex = normalizedKey.length > 0 ? hostView.hierarchyModelIndexForKey(normalizedKey) : -1;")));
    QVERIFY(sidebarSource.contains(QStringLiteral("renameController.beginRenameHierarchyItemKeyWhenVisible(normalizedKey, normalizedIndex, nextAttempts);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyItemMatchesModelIndex(item, index)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("const expectedKey = sidebarHierarchyView.hierarchyModelItemKeyAt(resolvedIndex);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("return sidebarHierarchyView.hierarchyItemKeyForVisualItem(item) === expectedKey;")));

    const qsizetype beginFunctionIndex = sidebarSource.indexOf(QStringLiteral("function beginRenameHierarchyItem(renameIndex)"));
    QVERIFY(beginFunctionIndex >= 0);
    const qsizetype editingAssignmentIndex = sidebarSource.indexOf(QStringLiteral("hostView.editingHierarchyIndex = renameIndex;"), beginFunctionIndex);
    const qsizetype presentationGuardIndex = sidebarSource.indexOf(QStringLiteral("if (!renameController.renamePresentationReady(renameIndex))"), editingAssignmentIndex);
    const qsizetype labelSeedIndex = sidebarSource.indexOf(QStringLiteral("hostView.editingHierarchyLabel = renameController.selectedHierarchyItemLabel();"), presentationGuardIndex);
    const qsizetype focusScheduleIndex = sidebarSource.indexOf(QStringLiteral("renameController.scheduleHierarchyRenameFieldFocus(renameIndex, 3);"), labelSeedIndex);
    const qsizetype beginFunctionEndIndex = sidebarSource.indexOf(QStringLiteral("function beginRenameHierarchyItemWhenVisible(renameIndex, remainingAttempts)"), beginFunctionIndex);
    QVERIFY(editingAssignmentIndex > beginFunctionIndex);
    QVERIFY(presentationGuardIndex > editingAssignmentIndex);
    QVERIFY(labelSeedIndex > presentationGuardIndex);
    QVERIFY(focusScheduleIndex > labelSeedIndex);
    QVERIFY(beginFunctionEndIndex > beginFunctionIndex);
    const QString beginFunctionBlock = sidebarSource.mid(beginFunctionIndex, beginFunctionEndIndex - beginFunctionIndex);
    QVERIFY(!beginFunctionBlock.contains(QStringLiteral("hostView.syncDisplayedHierarchyModel(true);")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("projectedItem.label = \" \";")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function focusHierarchyRenameField(renameIndex)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyRenameField.forceInputFocus();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyRenameField.inputItem.forceActiveFocus();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function scheduleHierarchyRenameFieldFocus(renameIndex, remainingPasses)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("renameController.scheduleHierarchyRenameFieldFocus(renameIndex, nextPasses);")));

    const qsizetype syncSelectedFunctionIndex = sidebarSource.indexOf(QStringLiteral("function syncSelectedHierarchyItem(focusView)"));
    QVERIFY(syncSelectedFunctionIndex >= 0);
    const qsizetype syncSelectedEndIndex = sidebarSource.indexOf(QStringLiteral("function updateNoteDropPreviewAtPosition"), syncSelectedFunctionIndex);
    QVERIFY(syncSelectedEndIndex > syncSelectedFunctionIndex);
    const QString syncSelectedBlock = sidebarSource.mid(syncSelectedFunctionIndex, syncSelectedEndIndex - syncSelectedFunctionIndex);
    QVERIFY(syncSelectedBlock.contains(QStringLiteral("sidebarHierarchyView.clearActiveHierarchyFocus();")));
    QVERIFY(syncSelectedBlock.contains(QStringLiteral("if (focusView && !sidebarHierarchyView.renameEditingActive)")));
    QVERIFY(!syncSelectedBlock.contains(QStringLiteral("if (focusView)\n            sidebarHierarchyView.forceActiveFocus();")));

    const qsizetype refreshFunctionIndex = sidebarSource.indexOf(QStringLiteral("function refreshEditingHierarchyPresentation(forceSelectionSync)"));
    QVERIFY(refreshFunctionIndex >= 0);
    const qsizetype itemKeyIndex = sidebarSource.indexOf(QStringLiteral("const itemKey = sidebarHierarchyView.hierarchyModelItemKeyAt(editingIndex);"), refreshFunctionIndex);
    const qsizetype activateByKeyIndex = sidebarSource.indexOf(QStringLiteral("hierarchyTree.activateListItemByKey(itemKey);"), itemKeyIndex);
    QVERIFY(itemKeyIndex > refreshFunctionIndex);
    QVERIFY(activateByKeyIndex > itemKeyIndex);
    const qsizetype resolveFunctionIndex = sidebarSource.indexOf(QStringLiteral("function resolveVisibleHierarchyItem(itemId)"));
    QVERIFY(resolveFunctionIndex >= 0);
    const qsizetype resolveFunctionEndIndex = sidebarSource.indexOf(QStringLiteral("function scheduleBookmarkPaletteVisualRefresh()"), resolveFunctionIndex);
    QVERIFY(resolveFunctionEndIndex > resolveFunctionIndex);
    const QString resolveFunctionBlock = sidebarSource.mid(resolveFunctionIndex, resolveFunctionEndIndex - resolveFunctionIndex);
    QVERIFY(!resolveFunctionBlock.contains(QStringLiteral("hierarchyTree.activeListItem")));
    QVERIFY(!resolveFunctionBlock.contains(QStringLiteral("activeListItemId")));
    const qsizetype locatorFunctionIndex = sidebarSource.indexOf(QStringLiteral("function hierarchyItemForResolvedIndex(itemId)"));
    QVERIFY(locatorFunctionIndex >= 0);
    const qsizetype locatorMatchIndex = sidebarSource.indexOf(QStringLiteral("hostView.hierarchyItemMatchesModelIndex(child, resolvedIndex)"), locatorFunctionIndex);
    QVERIFY(locatorMatchIndex > locatorFunctionIndex);

    const qsizetype deleteFunctionIndex = sidebarSource.indexOf(QStringLiteral("function requestDeleteFolder(targetIndex, reason)"), createFunctionIndex);
    QVERIFY(deleteFunctionIndex > createFunctionIndex);
    const QString createBlock = sidebarSource.mid(createFunctionIndex, deleteFunctionIndex - createFunctionIndex);
    QVERIFY(!createBlock.contains(QStringLiteral("beginRenameSelectedHierarchyItem()")));

    const qsizetype footerFunctionIndex = sidebarSource.indexOf(QStringLiteral("function requestHierarchyFooterAction(action)"), deleteFunctionIndex);
    QVERIFY(footerFunctionIndex > deleteFunctionIndex);
    const QString deleteBlock = sidebarSource.mid(deleteFunctionIndex, footerFunctionIndex - deleteFunctionIndex);
    const qsizetype deleteBridgeIndex = deleteBlock.indexOf(QStringLiteral("sidebarHierarchyView.hierarchyInteractionBridge.deleteSelectedFolder();"));
    QVERIFY(deleteBridgeIndex >= 0);
    const qsizetype selectionSyncIndex = deleteBlock.indexOf(QStringLiteral("sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();"), deleteBridgeIndex);
    const qsizetype immediateFocusSyncIndex = deleteBlock.indexOf(QStringLiteral("sidebarHierarchyView.syncSelectedHierarchyItem(false);"), deleteBridgeIndex);
    const qsizetype deferredFocusSyncIndex = deleteBlock.indexOf(QStringLiteral("sidebarHierarchyView.scheduleSelectedHierarchySync(false);"), deleteBridgeIndex);
    QVERIFY(selectionSyncIndex > deleteBridgeIndex);
    QVERIFY(immediateFocusSyncIndex > selectionSyncIndex);
    QVERIFY(deferredFocusSyncIndex > immediateFocusSyncIndex);
    QVERIFY(!deleteBlock.contains(QStringLiteral("syncSelectedHierarchyItem(true)")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_noteDropSurfaceDoesNotInterceptHierarchyItemDrags()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));
    const QString listBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(!listBarSource.isEmpty());
    QVERIFY(listBarSource.contains(QStringLiteral("Drag.keys: [\"whatson.library.note\"]")));

    const qsizetype noteDropSurfaceIndex = sidebarSource.indexOf(QStringLiteral("id: noteDropSurface"));
    QVERIFY(noteDropSurfaceIndex >= 0);

    const qsizetype dropKeysIndex = sidebarSource.indexOf(QStringLiteral("keys: [\"whatson.library.note\"]"), noteDropSurfaceIndex);
    const qsizetype emptyPayloadGuardIndex = sidebarSource.indexOf(QStringLiteral("if (noteIds.length <= 0)"), noteDropSurfaceIndex);
    const qsizetype hierarchyReorderIndex = sidebarSource.indexOf(QStringLiteral("applyHierarchyReorder(sidebarHierarchyView.hierarchyReorderCommitModel(), itemKey)"));

    QVERIFY(dropKeysIndex > noteDropSurfaceIndex);
    QVERIFY(emptyPayloadGuardIndex > noteDropSurfaceIndex);
    QVERIFY(hierarchyReorderIndex >= 0);
    QVERIFY(!sidebarSource.contains(QStringLiteral("applyHierarchyMove(fromIndex, toIndex, depth, itemKey)")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_chevronHitTestUsesLvrsChevronSlotContentItem()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());

    const qsizetype slotFunctionIndex = sidebarSource.indexOf(QStringLiteral("function hierarchyItemChevronSlot(item)"));
    QVERIFY(slotFunctionIndex >= 0);
    const qsizetype slotFunctionEndIndex =
        sidebarSource.indexOf(QStringLiteral("function invalidateHierarchySelectionVisuals()"), slotFunctionIndex);
    QVERIFY(slotFunctionEndIndex > slotFunctionIndex);
    const QString slotFunctionBlock = sidebarSource.mid(slotFunctionIndex, slotFunctionEndIndex - slotFunctionIndex);

    QVERIFY(slotFunctionBlock.contains(QStringLiteral("objectName === \"hierarchyItemChevron\"")));
    QVERIFY(slotFunctionBlock.contains(QStringLiteral("item.contentItem")));
    QVERIFY(slotFunctionBlock.contains(QStringLiteral("appendChevronSlotChildren(item.contentItem && item.contentItem.children")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_doesNotOverlayLvrsChevronClicks()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));
    const QString layoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/HierarchySidebarLayout.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(!layoutSource.isEmpty());

    QVERIFY(layoutSource.contains(QStringLiteral("hierarchyEditable: hierarchyDragDropBridge.reorderContractAvailable")));

    const qsizetype hierarchyTreeIndex = sidebarSource.indexOf(QStringLiteral("LV.Hierarchy {"));
    QVERIFY(hierarchyTreeIndex >= 0);
    const qsizetype editableBindingIndex = sidebarSource.indexOf(QStringLiteral("editable: sidebarHierarchyView.hierarchyEditable"), hierarchyTreeIndex);
    const qsizetype expandedHandlerIndex = sidebarSource.indexOf(QStringLiteral("onListItemExpanded: function (item, itemId, index, expanded)"), hierarchyTreeIndex);
    const qsizetype expansionCommitIndex = sidebarSource.indexOf(QStringLiteral("hierarchyInteractionController.handleExpansionSignal(item, resolvedExpansionIndex, expanded)"), expandedHandlerIndex);
    const qsizetype leftTapHandlerIndex = sidebarSource.indexOf(QStringLiteral("id: hierarchyLeftPressTapHandler"), hierarchyTreeIndex);
    const qsizetype movedHandlerIndex = sidebarSource.indexOf(QStringLiteral("onListItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth)"), hierarchyTreeIndex);
    const qsizetype reorderCallIndex = sidebarSource.indexOf(QStringLiteral("applyHierarchyReorder(sidebarHierarchyView.hierarchyReorderCommitModel(), itemKey)"), movedHandlerIndex);
    QVERIFY(editableBindingIndex > hierarchyTreeIndex);
    QVERIFY(expandedHandlerIndex > editableBindingIndex);
    QVERIFY(expansionCommitIndex > expandedHandlerIndex);
    QVERIFY(leftTapHandlerIndex > expandedHandlerIndex);
    QVERIFY(movedHandlerIndex > editableBindingIndex);
    QVERIFY(reorderCallIndex > movedHandlerIndex);

    QVERIFY(sidebarSource.contains(QStringLiteral("function armHierarchyUserExpansionAtPosition(x, y)")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("id: hierarchyChevronPointerSurface")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function beginHierarchyChevronPointerPress")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function finishHierarchyChevronPointerPress")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("mouse.accepted = sidebarHierarchyView.beginHierarchyChevronPointerPress")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("preventStealing: sidebarHierarchyView.hierarchyChevronPointerPressKey.length > 0")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_chevronTapFallbackScopesCommitToPressedItem()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(sidebarSource.contains(QStringLiteral("property var hierarchyChevronPointerPressItem: null")));
    QVERIFY(sidebarSource.contains(QStringLiteral("property int hierarchyChevronPointerPressIndex: -1")));

    const qsizetype armFunctionIndex = sidebarSource.indexOf(QStringLiteral("function armHierarchyUserExpansionAtPosition(x, y)"));
    QVERIFY(armFunctionIndex >= 0);
    const qsizetype armFunctionEndIndex =
        sidebarSource.indexOf(QStringLiteral("function resolveHierarchyActivationIndex"), armFunctionIndex);
    QVERIFY(armFunctionEndIndex > armFunctionIndex);
    const QString armFunctionBlock = sidebarSource.mid(armFunctionIndex, armFunctionEndIndex - armFunctionIndex);
    QVERIFY(armFunctionBlock.contains(QStringLiteral("const target = sidebarHierarchyView.hierarchyChevronExpansionTargetAtPosition(x, y);")));
    QVERIFY(armFunctionBlock.contains(QStringLiteral("sidebarHierarchyView.hierarchyChevronPointerPressItem = target.item;")));
    QVERIFY(armFunctionBlock.contains(QStringLiteral("sidebarHierarchyView.hierarchyChevronPointerPressIndex = target.index;")));

    const qsizetype tappedHandlerIndex = sidebarSource.indexOf(QStringLiteral("onTapped: function (eventPoint, button)"));
    QVERIFY(tappedHandlerIndex >= 0);
    const qsizetype tappedHandlerEndIndex =
        sidebarSource.indexOf(QStringLiteral("TapHandler {\n            id: hierarchyContextMenuTapHandler"), tappedHandlerIndex);
    QVERIFY(tappedHandlerEndIndex > tappedHandlerIndex);
    const QString tappedHandlerBlock = sidebarSource.mid(tappedHandlerIndex, tappedHandlerEndIndex - tappedHandlerIndex);

    QVERIFY(tappedHandlerBlock.contains(QStringLiteral("const pressedItem = sidebarHierarchyView.hierarchyChevronPointerPressItem;")));
    QVERIFY(tappedHandlerBlock.contains(QStringLiteral("const pressedIndex = sidebarHierarchyView.hierarchyChevronPointerPressIndex;")));
    QVERIFY(tappedHandlerBlock.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyChevronExpansionForTarget(pressedItem, pressedIndex, armedKey)")));
    QVERIFY(!tappedHandlerBlock.contains(QStringLiteral("hierarchyChevronExpansionTargetAtPosition(tapX, tapY)")));
    QVERIFY(!tappedHandlerBlock.contains(QStringLiteral("requestHierarchyChevronExpansionAtPosition(tapX, tapY, armedKey)")));

    const qsizetype targetFunctionIndex =
        sidebarSource.indexOf(QStringLiteral("function requestHierarchyChevronExpansionForTarget(targetItem, targetIndex, expectedKey)"));
    QVERIFY(targetFunctionIndex >= 0);
    const qsizetype targetFunctionEndIndex =
        sidebarSource.indexOf(QStringLiteral("function requestHierarchyChevronExpansionAtPosition"), targetFunctionIndex);
    QVERIFY(targetFunctionEndIndex > targetFunctionIndex);
    const QString targetFunctionBlock = sidebarSource.mid(targetFunctionIndex, targetFunctionEndIndex - targetFunctionIndex);
    QVERIFY(!targetFunctionBlock.contains(QStringLiteral("syncDisplayedHierarchyModel(true)")));
    QVERIFY(!targetFunctionBlock.contains(QStringLiteral("requestViewHook(\"hierarchy.chevron.toggle\")")));
    QVERIFY(!targetFunctionBlock.contains(QStringLiteral("hierarchyChevronExpansionTargetAtPosition")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_routesFooterActionsDirectlyFromQml()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));
    const QString layoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/HierarchySidebarLayout.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(!layoutSource.isEmpty());
    const qsizetype footerButtonsIndex = sidebarSource.indexOf(QStringLiteral("readonly property var hierarchyFooterToolbarButtons"));
    QVERIFY(footerButtonsIndex >= 0);
    const qsizetype footerCreateEventIndex = sidebarSource.indexOf(QStringLiteral("eventName: \"hierarchy.footer.create\""), footerButtonsIndex);
    const qsizetype footerDeleteEventIndex = sidebarSource.indexOf(QStringLiteral("eventName: \"hierarchy.footer.delete\""), footerButtonsIndex);
    const qsizetype footerOptionsEventIndex = sidebarSource.indexOf(QStringLiteral("eventName: \"hierarchy.footer.options\""), footerButtonsIndex);
    QVERIFY(footerCreateEventIndex >= 0);
    QVERIFY(footerDeleteEventIndex > footerCreateEventIndex);
    QVERIFY(footerOptionsEventIndex > footerDeleteEventIndex);
    QVERIFY(sidebarSource.contains(QStringLiteral("function handleHierarchyFooterButtonClicked(index, config)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyFooterActionName(index, eventName)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function requestHierarchyFooterAction(action)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("property string hierarchyFooterTriggerQueuedAction: \"\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("onButtonClicked: function (index, config)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(\"hierarchy.footer.create\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(\"hierarchy.footer.delete\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(\"hierarchy.footer.options\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("const action = sidebarHierarchyView.hierarchyFooterActionName(index, normalizedEventName);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("return sidebarHierarchyView.requestHierarchyFooterAction(action);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("committed = sidebarHierarchyView.requestCreateFolder();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("committed = sidebarHierarchyView.requestDeleteFolder();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestViewOptions();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("if (sidebarHierarchyView.hierarchyFooterTriggerQueuedAction === normalizedAction)")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("hierarchyInteractionController.requestFooterAction")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("hierarchyInteractionController.footerActionName")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onFooterCreateRequested")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onFooterDeleteRequested")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onFooterOptionsRequested")));
    QVERIFY(layoutSource.contains(QStringLiteral("SidebarHierarchyInteractionController")));
    QVERIFY(layoutSource.contains(QStringLiteral("hierarchyInteractionController: sidebarHierarchyInteractionController")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.create\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.delete\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.options\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("iconName: \"generaladd\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("iconName: \"generaldelete\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("iconName: \"generalmoreHorizontal\"")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("iconName: \"generalsettings\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("button1: sidebarHierarchyView.hierarchyFooterToolbarButtons[0]")));
    QVERIFY(sidebarSource.contains(QStringLiteral("button2: sidebarHierarchyView.hierarchyFooterToolbarButtons[1]")));
    QVERIFY(sidebarSource.contains(QStringLiteral("button3: sidebarHierarchyView.hierarchyFooterToolbarButtons[2]")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestCreateFolder();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestDeleteFolder();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestViewOptions();")));
}
