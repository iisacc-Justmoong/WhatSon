pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property int selectionAnchorIndex: -1
    property var selectedIndices: []

    function normalizedKeyboardModifiers(modifiers) {
        return modifiers === undefined || modifiers === null
                ? Qt.NoModifier
                : modifiers;
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

    function resolveSelectionModifiers(modifiers, cachedModifiers, cachedCapturedAtMs) {
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (controller.selectionModifierPressed(normalizedModifiers))
            return normalizedModifiers;
        const capturedAtMs = Number(cachedCapturedAtMs);
        const cacheAgeMs = Date.now() - capturedAtMs;
        const cacheFresh = capturedAtMs > 0 && isFinite(cacheAgeMs) && cacheAgeMs >= 0 && cacheAgeMs <= 800;
        const normalizedCachedModifiers = controller.normalizedKeyboardModifiers(cachedModifiers);
        if (cacheFresh && controller.selectionModifierPressed(normalizedCachedModifiers))
            return normalizedCachedModifiers;
        return normalizedModifiers;
    }

    function normalizeSelectedNoteIndices(indices) {
        if (!indices || indices.length === undefined || !controller.view)
            return [];
        const normalized = [];
        for (let row = 0; row < indices.length; ++row) {
            const normalizedIndex = controller.view.normalizeCurrentIndex(indices[row]);
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

    function noteSelectionContainsIndex(index) {
        if (!controller.view)
            return false;
        const normalizedIndex = controller.view.normalizeCurrentIndex(index);
        if (normalizedIndex < 0)
            return false;
        const normalizedSelection = controller.normalizeSelectedNoteIndices(controller.selectedIndices);
        return normalizedSelection.indexOf(normalizedIndex) >= 0;
    }

    function setSelectedNoteIndices(indices) {
        controller.selectedIndices = controller.normalizeSelectedNoteIndices(indices);
    }

    function selectionRangeIndices(anchorIndex, targetIndex) {
        if (!controller.view)
            return [];
        const normalizedAnchor = controller.view.normalizeCurrentIndex(anchorIndex);
        const normalizedTarget = controller.view.normalizeCurrentIndex(targetIndex);
        if (normalizedTarget < 0)
            return [];
        if (normalizedAnchor < 0)
            return [normalizedTarget];
        const begin = Math.min(normalizedAnchor, normalizedTarget);
        const end = Math.max(normalizedAnchor, normalizedTarget);
        const range = [];
        for (let candidate = begin; candidate <= end; ++candidate)
            range.push(candidate);
        return range;
    }

    function syncSelectionFromCommittedState() {
        if (!controller.view)
            return;
        const normalizedIndex = controller.view.normalizeCurrentIndex(
                    controller.view.currentIndexFromModel());
        if (normalizedIndex < 0) {
            controller.setSelectedNoteIndices([]);
            controller.selectionAnchorIndex = -1;
            return;
        }
        controller.setSelectedNoteIndices([normalizedIndex]);
        controller.selectionAnchorIndex = normalizedIndex;
    }

    function requestNoteSelection(index, noteId, modifiers) {
        if (!controller.view)
            return;
        const normalizedIndex = controller.view.normalizeCurrentIndex(index);
        if (normalizedIndex < 0)
            return;
        const normalizedModifiers = controller.normalizedKeyboardModifiers(modifiers);
        if (controller.selectionRangeModifierPressed(normalizedModifiers)) {
            let anchorIndex = controller.view.normalizeCurrentIndex(controller.selectionAnchorIndex);
            if (anchorIndex < 0)
                anchorIndex = controller.view.normalizeCurrentIndex(controller.view.currentIndexFromModel());
            if (anchorIndex < 0)
                anchorIndex = normalizedIndex;
            const rangeSelection = controller.selectionRangeIndices(anchorIndex, normalizedIndex);
            if (controller.selectionToggleModifierPressed(normalizedModifiers)) {
                const selectedIndices = controller.normalizeSelectedNoteIndices(controller.selectedIndices);
                for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)
                    selectedIndices.push(rangeSelection[selectionIndex]);
                controller.setSelectedNoteIndices(selectedIndices);
            } else {
                controller.setSelectedNoteIndices(rangeSelection);
            }
            controller.selectionAnchorIndex = anchorIndex;
            controller.view.activateNoteIndex(normalizedIndex, noteId);
            return;
        }
        if (controller.selectionToggleModifierPressed(normalizedModifiers)) {
            const selectedIndices = controller.normalizeSelectedNoteIndices(controller.selectedIndices);
            const existingSelectionIndex = selectedIndices.indexOf(normalizedIndex);
            if (existingSelectionIndex < 0) {
                selectedIndices.push(normalizedIndex);
                controller.setSelectedNoteIndices(selectedIndices);
                controller.selectionAnchorIndex = normalizedIndex;
                controller.view.activateNoteIndex(normalizedIndex, noteId);
                return;
            }
            if (selectedIndices.length <= 1) {
                controller.setSelectedNoteIndices([normalizedIndex]);
                controller.selectionAnchorIndex = normalizedIndex;
                controller.view.activateNoteIndex(normalizedIndex, noteId);
                return;
            }
            selectedIndices.splice(existingSelectionIndex, 1);
            controller.setSelectedNoteIndices(selectedIndices);
            const committedIndex = controller.view.normalizeCurrentIndex(controller.view.currentIndexFromModel());
            const committedSelectionRetained = controller.noteSelectionContainsIndex(committedIndex);
            if (committedSelectionRetained)
                return;
            const fallbackIndex = selectedIndices.length > 0 ? selectedIndices[selectedIndices.length - 1] : -1;
            if (fallbackIndex >= 0)
                controller.view.activateNoteIndex(fallbackIndex, "");
            return;
        }
        controller.setSelectedNoteIndices([normalizedIndex]);
        controller.selectionAnchorIndex = normalizedIndex;
        controller.view.activateNoteIndex(normalizedIndex, noteId);
    }
}
