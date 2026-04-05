pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var section: null
    property int selectionAnchorIndex: -1
    property var selectedIndices: []
    property int pointerSelectionModifiers: Qt.NoModifier
    property double pointerSelectionModifiersCapturedAtMs: 0

    function itemCount() {
        if (!section || !section.listItems)
            return 0;
        if (section.listItems.length !== undefined)
            return Math.max(0, Number(section.listItems.length) || 0);
        if (section.listItems.count !== undefined)
            return Math.max(0, Number(section.listItems.count) || 0);
        return 0;
    }

    function normalizeIndex(value) {
        const numericValue = Number(value);
        const count = controller.itemCount();
        if (!isFinite(numericValue) || numericValue < 0 || count <= 0)
            return -1;
        if (numericValue >= count)
            return count - 1;
        return Math.floor(numericValue);
    }

    function normalizedKeyboardModifiers(modifiers) {
        const eventModifiers = modifiers === undefined || modifiers === null
                ? Qt.NoModifier
                : modifiers;
        const applicationModifiers = Qt.application && Qt.application.keyboardModifiers !== undefined
                ? Qt.application.keyboardModifiers
                : Qt.NoModifier;
        return eventModifiers | applicationModifiers;
    }

    function selectionToggleModifierPressed(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        const toggleMask = Qt.ControlModifier | Qt.MetaModifier;
        return Boolean(normalizedModifiers & toggleMask);
    }

    function selectionRangeModifierPressed(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        return Boolean(normalizedModifiers & Qt.ShiftModifier);
    }

    function selectionModifierPressed(modifiers) {
        return controller.selectionRangeModifierPressed(modifiers)
                || controller.selectionToggleModifierPressed(modifiers);
    }

    function capturePointerSelectionModifiers(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (!controller.selectionModifierPressed(normalizedModifiers))
            return;
        controller.pointerSelectionModifiers = normalizedModifiers;
        controller.pointerSelectionModifiersCapturedAtMs = Date.now();
    }

    function clearPointerSelectionModifiers() {
        controller.pointerSelectionModifiers = Qt.NoModifier;
        controller.pointerSelectionModifiersCapturedAtMs = 0;
    }

    function resolveSelectionModifiers(modifiers) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (controller.selectionModifierPressed(normalizedModifiers))
            return normalizedModifiers;
        const capturedAtMs = Number(controller.pointerSelectionModifiersCapturedAtMs);
        const cacheAgeMs = Date.now() - capturedAtMs;
        const cacheFresh = capturedAtMs > 0 && isFinite(cacheAgeMs) && cacheAgeMs >= 0 && cacheAgeMs <= 800;
        const normalizedCachedModifiers = controller.normalizedKeyboardModifiers(controller.pointerSelectionModifiers);
        if (cacheFresh && controller.selectionModifierPressed(normalizedCachedModifiers))
            return normalizedCachedModifiers;
        return normalizedModifiers;
    }

    function normalizeSelectionIndices(indices) {
        if (!indices || indices.length === undefined)
            return [];
        const normalized = [];
        for (let index = 0; index < indices.length; ++index) {
            const normalizedIndex = controller.normalizeIndex(indices[index]);
            if (normalizedIndex < 0)
                continue;
            if (normalized.indexOf(normalizedIndex) >= 0)
                continue;
            normalized.push(normalizedIndex);
        }
        normalized.sort(function (left, right) {
            return left - right;
        });
        return normalized;
    }

    function setSelectedIndices(indices) {
        controller.selectedIndices = controller.normalizeSelectionIndices(indices);
    }

    function selectionContainsIndex(index) {
        const normalizedIndex = controller.normalizeIndex(index);
        if (normalizedIndex < 0)
            return false;
        return controller.normalizeSelectionIndices(controller.selectedIndices).indexOf(normalizedIndex) >= 0;
    }

    function selectionRangeIndices(anchorIndex, targetIndex) {
        const normalizedAnchor = controller.normalizeIndex(anchorIndex);
        const normalizedTarget = controller.normalizeIndex(targetIndex);
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

    function activateIndex(index) {
        if (!section)
            return;
        const normalizedIndex = controller.normalizeIndex(index);
        if (normalizedIndex < 0)
            return;
        section.itemTriggered(normalizedIndex);
    }

    function reconcileSelection() {
        const committedIndex = controller.normalizeIndex(section ? section.activeIndex : -1);
        const normalizedSelection = controller.normalizeSelectionIndices(controller.selectedIndices);
        if (committedIndex < 0) {
            if (normalizedSelection.length > 0)
                controller.setSelectedIndices([]);
            controller.selectionAnchorIndex = -1;
            return;
        }
        if (normalizedSelection.length === 0 || normalizedSelection.indexOf(committedIndex) < 0)
            controller.setSelectedIndices([committedIndex]);
        else
            controller.setSelectedIndices(normalizedSelection);
        const normalizedAnchorIndex = controller.normalizeIndex(controller.selectionAnchorIndex);
        if (normalizedAnchorIndex < 0 || !controller.selectionContainsIndex(normalizedAnchorIndex))
            controller.selectionAnchorIndex = committedIndex;
    }

    function requestSelection(index, modifiers) {
        const normalizedIndex = controller.normalizeIndex(index);
        if (normalizedIndex < 0)
            return;
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (controller.selectionRangeModifierPressed(normalizedModifiers)) {
            let anchorIndex = controller.normalizeIndex(controller.selectionAnchorIndex);
            if (anchorIndex < 0)
                anchorIndex = controller.normalizeIndex(section ? section.activeIndex : -1);
            if (anchorIndex < 0)
                anchorIndex = normalizedIndex;
            const rangeSelection = controller.selectionRangeIndices(anchorIndex, normalizedIndex);
            if (controller.selectionToggleModifierPressed(normalizedModifiers)) {
                const selectedIndices = controller.normalizeSelectionIndices(controller.selectedIndices);
                for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)
                    selectedIndices.push(rangeSelection[selectionIndex]);
                controller.setSelectedIndices(selectedIndices);
            } else {
                controller.setSelectedIndices(rangeSelection);
            }
            controller.selectionAnchorIndex = anchorIndex;
            controller.activateIndex(normalizedIndex);
            return;
        }
        if (controller.selectionToggleModifierPressed(normalizedModifiers)) {
            const selectedIndices = controller.normalizeSelectionIndices(controller.selectedIndices);
            const existingSelectionIndex = selectedIndices.indexOf(normalizedIndex);
            if (existingSelectionIndex < 0) {
                selectedIndices.push(normalizedIndex);
                controller.setSelectedIndices(selectedIndices);
                controller.selectionAnchorIndex = normalizedIndex;
                controller.activateIndex(normalizedIndex);
                return;
            }
            if (selectedIndices.length <= 1) {
                controller.setSelectedIndices([normalizedIndex]);
                controller.selectionAnchorIndex = normalizedIndex;
                controller.activateIndex(normalizedIndex);
                return;
            }
            selectedIndices.splice(existingSelectionIndex, 1);
            controller.setSelectedIndices(selectedIndices);
            const committedIndex = controller.normalizeIndex(section ? section.activeIndex : -1);
            if (controller.selectionContainsIndex(committedIndex)) {
                controller.selectionAnchorIndex = normalizedIndex;
                return;
            }
            const fallbackIndex = selectedIndices.length > 0 ? selectedIndices[selectedIndices.length - 1] : -1;
            controller.selectionAnchorIndex = fallbackIndex;
            if (fallbackIndex >= 0)
                controller.activateIndex(fallbackIndex);
            return;
        }
        controller.setSelectedIndices([normalizedIndex]);
        controller.selectionAnchorIndex = normalizedIndex;
        controller.activateIndex(normalizedIndex);
    }
}
