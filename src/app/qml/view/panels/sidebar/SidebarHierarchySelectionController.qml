pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property int selectionAnchorIndex: -1
    property var selectedIndices: []
    property int pointerSelectionModifiers: Qt.NoModifier
    property double pointerSelectionModifiersCapturedAtMs: 0

    function normalizedKeyboardModifiers(modifiers) {
        const eventModifiers = modifiers === undefined || modifiers === null
                ? Qt.NoModifier
                : modifiers;
        const applicationModifiers = Qt.application && Qt.application.keyboardModifiers !== undefined
                ? Qt.application.keyboardModifiers
                : Qt.NoModifier;
        return eventModifiers | applicationModifiers;
    }

    function hierarchySelectionToggleModifierPressed(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        const toggleMask = Qt.ControlModifier | Qt.MetaModifier;
        return Boolean(normalizedModifiers & toggleMask);
    }

    function hierarchySelectionRangeModifierPressed(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        return Boolean(normalizedModifiers & Qt.ShiftModifier);
    }

    function hierarchySelectionModifierPressed(modifiers) {
        return controller.hierarchySelectionRangeModifierPressed(modifiers)
                || controller.hierarchySelectionToggleModifierPressed(modifiers);
    }

    function captureHierarchyPointerSelectionModifiers(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (!controller.hierarchySelectionModifierPressed(normalizedModifiers))
            return;
        controller.pointerSelectionModifiers = normalizedModifiers;
        controller.pointerSelectionModifiersCapturedAtMs = Date.now();
    }

    function clearHierarchyPointerSelectionModifiers() {
        controller.pointerSelectionModifiers = Qt.NoModifier;
        controller.pointerSelectionModifiersCapturedAtMs = 0;
    }

    function resolveHierarchySelectionModifiers(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (controller.hierarchySelectionModifierPressed(normalizedModifiers))
            return normalizedModifiers;
        const capturedAtMs = Number(controller.pointerSelectionModifiersCapturedAtMs);
        const cacheAgeMs = Date.now() - capturedAtMs;
        const cacheFresh = capturedAtMs > 0 && isFinite(cacheAgeMs) && cacheAgeMs >= 0 && cacheAgeMs <= 800;
        const normalizedCachedModifiers = controller.normalizedKeyboardModifiers(controller.pointerSelectionModifiers);
        if (cacheFresh && controller.hierarchySelectionModifierPressed(normalizedCachedModifiers))
            return normalizedCachedModifiers;
        return normalizedModifiers;
    }

    function normalizeHierarchySelectionIndices(indices) {
        if (!indices || indices.length === undefined || !controller.view)
            return [];
        const normalized = [];
        for (let index = 0; index < indices.length; ++index) {
            const resolvedIndex = controller.view.normalizedInteger(indices[index], -1);
            if (resolvedIndex < 0)
                continue;
            if (normalized.indexOf(resolvedIndex) >= 0)
                continue;
            normalized.push(resolvedIndex);
        }
        normalized.sort(function (left, right) {
            return left - right;
        });
        return normalized;
    }

    function setSelectedHierarchyIndices(indices) {
        controller.selectedIndices = controller.normalizeHierarchySelectionIndices(indices);
        if (controller.view)
            controller.view.invalidateHierarchySelectionVisuals();
    }

    function hierarchySelectionContainsIndex(index) {
        if (!controller.view)
            return false;
        const resolvedIndex = controller.view.normalizedInteger(index, -1);
        if (resolvedIndex < 0)
            return false;
        const normalizedSelection = controller.normalizeHierarchySelectionIndices(controller.selectedIndices);
        return normalizedSelection.indexOf(resolvedIndex) >= 0;
    }

    function hierarchySelectionRangeIndices(anchorIndex, targetIndex) {
        if (!controller.view)
            return [];
        const normalizedAnchor = controller.view.normalizedInteger(anchorIndex, -1);
        const normalizedTarget = controller.view.normalizedInteger(targetIndex, -1);
        if (normalizedTarget < 0)
            return [];
        if (normalizedAnchor < 0)
            return [normalizedTarget];
        const begin = Math.min(normalizedAnchor, normalizedTarget);
        const end = Math.max(normalizedAnchor, normalizedTarget);
        const range = [];
        for (let index = begin; index <= end; ++index)
            range.push(index);
        return range;
    }

    function syncHierarchySelectionFromSelectedFolder() {
        if (!controller.view)
            return;
        const selectedIndex = controller.view.normalizedInteger(controller.view.selectedFolderIndex, -1);
        if (selectedIndex < 0) {
            controller.setSelectedHierarchyIndices([]);
            controller.selectionAnchorIndex = -1;
            return;
        }
        controller.setSelectedHierarchyIndices([selectedIndex]);
        controller.selectionAnchorIndex = selectedIndex;
    }

    function emitHierarchySelectionActivation(item, resolvedIndex) {
        if (!controller.view || !controller.view.hierarchyViewModel)
            return;
        const normalizedIndex = controller.view.normalizedInteger(resolvedIndex, -1);
        if (normalizedIndex < 0)
            return;
        const activationItem = item ? item : controller.view.resolveVisibleHierarchyItem(normalizedIndex);
        controller.view.hierarchyViewModel.setHierarchySelectedIndex(normalizedIndex);
        controller.view.hierarchyItemActivated(activationItem, normalizedIndex, normalizedIndex);
    }

    function requestHierarchySelection(item, resolvedIndex, modifiers) {
        if (!controller.view || !controller.view.hierarchyViewModel)
            return;
        const normalizedIndex = controller.view.normalizedInteger(resolvedIndex, -1);
        if (normalizedIndex < 0)
            return;
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (controller.hierarchySelectionRangeModifierPressed(normalizedModifiers)) {
            let anchorIndex = controller.view.normalizedInteger(controller.selectionAnchorIndex, -1);
            if (anchorIndex < 0)
                anchorIndex = controller.view.normalizedInteger(controller.view.selectedFolderIndex, -1);
            if (anchorIndex < 0)
                anchorIndex = normalizedIndex;
            const rangeSelection = controller.hierarchySelectionRangeIndices(anchorIndex, normalizedIndex);
            if (controller.hierarchySelectionToggleModifierPressed(normalizedModifiers)) {
                const selectedIndices = controller.normalizeHierarchySelectionIndices(controller.selectedIndices);
                for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)
                    selectedIndices.push(rangeSelection[selectionIndex]);
                controller.setSelectedHierarchyIndices(selectedIndices);
            } else {
                controller.setSelectedHierarchyIndices(rangeSelection);
            }
            controller.selectionAnchorIndex = anchorIndex;
            controller.emitHierarchySelectionActivation(item, normalizedIndex);
            return;
        }
        if (controller.hierarchySelectionToggleModifierPressed(normalizedModifiers)) {
            const selectedIndices = controller.normalizeHierarchySelectionIndices(controller.selectedIndices);
            const existingSelectionIndex = selectedIndices.indexOf(normalizedIndex);
            if (existingSelectionIndex < 0) {
                selectedIndices.push(normalizedIndex);
                controller.setSelectedHierarchyIndices(selectedIndices);
                controller.selectionAnchorIndex = normalizedIndex;
                controller.emitHierarchySelectionActivation(item, normalizedIndex);
                return;
            }
            if (selectedIndices.length <= 1) {
                controller.setSelectedHierarchyIndices([normalizedIndex]);
                controller.selectionAnchorIndex = normalizedIndex;
                controller.emitHierarchySelectionActivation(item, normalizedIndex);
                return;
            }
            selectedIndices.splice(existingSelectionIndex, 1);
            controller.setSelectedHierarchyIndices(selectedIndices);
            const committedIndex = controller.view.normalizedInteger(controller.view.selectedFolderIndex, -1);
            if (controller.hierarchySelectionContainsIndex(committedIndex))
                return;
            const fallbackIndex = selectedIndices.length > 0 ? selectedIndices[selectedIndices.length - 1] : -1;
            if (fallbackIndex >= 0)
                controller.emitHierarchySelectionActivation(
                            controller.view.resolveVisibleHierarchyItem(fallbackIndex),
                            fallbackIndex);
            return;
        }
        controller.setSelectedHierarchyIndices([normalizedIndex]);
        controller.selectionAnchorIndex = normalizedIndex;
        controller.emitHierarchySelectionActivation(item, normalizedIndex);
    }
}
