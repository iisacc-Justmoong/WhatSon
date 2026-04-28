pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: model

    property var contentsView: null
    property var contextMenuCoordinator: null
    property var editorSelectionController: null
    property var editorViewport: null
    property var resourceImportController: null
    property var structuredDocumentFlow: null

    function eventRequestsPasteShortcut(event) {
        if (!event)
            return false;
        if (event.matches !== undefined && event.matches(StandardKey.Paste))
            return true;

        const modifiers = event.modifiers;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        if (!altPressed
                && !shiftPressed
                && (metaPressed || controlPressed)
                && (event.key === Qt.Key_V || normalizedText === "V")) {
            return true;
        }
        if (!metaPressed
                && !controlPressed
                && !altPressed
                && shiftPressed
                && event.key === Qt.Key_Insert) {
            return true;
        }
        return false;
    }

    function inlineFormatShortcutTag(event) {
        if (!event)
            return "";

        const modifiers = Number(event.modifiers) || 0;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const commandPressed = metaPressed || controlPressed;
        if (altPressed || !commandPressed)
            return "";

        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        const key = Number(event.key);
        if (!shiftPressed) {
            if (key === Qt.Key_B || normalizedText === "B")
                return "bold";
            if (key === Qt.Key_I || normalizedText === "I")
                return "italic";
            if (key === Qt.Key_U || normalizedText === "U")
                return "underline";
            return "";
        }
        if (key === Qt.Key_X || normalizedText === "X")
            return "strikethrough";
        if (key === Qt.Key_E || normalizedText === "E")
            return "highlight";
        return "";
    }

    function handleInlineFormatTagShortcut(event) {
        const tagName = model.inlineFormatShortcutTag(event);
        if (tagName.length === 0)
            return false;
        const handled = model.contentsView.queueInlineFormatWrap(tagName);
        if (handled && event)
            event.accepted = true;
        return handled;
    }

    function clipboardImageAvailableForPaste() {
        if (!model.contentsView.resourcesImportViewModel)
            return false;
        if (model.contentsView.resourcesImportViewModel.refreshClipboardImageAvailabilitySnapshot !== undefined)
            return !!model.contentsView.resourcesImportViewModel.refreshClipboardImageAvailabilitySnapshot();
        if (model.contentsView.resourcesImportViewModel.clipboardImageAvailable === undefined)
            return false;
        return !!model.contentsView.resourcesImportViewModel.clipboardImageAvailable;
    }

    function handleClipboardImagePasteShortcut(event) {
        if (!model.eventRequestsPasteShortcut(event))
            return false;
        if (!model.contentsView.resourcesImportViewModel
                || (model.contentsView.resourcesImportViewModel.busy !== undefined
                    && model.contentsView.resourcesImportViewModel.busy)
                || !model.clipboardImageAvailableForPaste()) {
            return false;
        }
        const pasted = model.resourceImportController.pasteClipboardImageAsResource();
        if (pasted && event)
            event.accepted = true;
        return pasted;
    }

    function handleTagManagementShortcutKeyPress(event) {
        if (model.handleClipboardImagePasteShortcut(event))
            return true;
        if (model.handleInlineFormatTagShortcut(event))
            return true;
        return false;
    }

    function handleSelectionContextMenuEvent(eventName) {
        if (model.handleStructuredSelectionContextMenuEvent(eventName))
            return;
        model.editorSelectionController.handleSelectionContextMenuEvent(eventName);
    }

    function encodeXmlAttributeValue(value) {
        let encodedValue = value === undefined || value === null ? "" : String(value);
        encodedValue = encodedValue.replace(/&/g, "&amp;");
        encodedValue = encodedValue.replace(/"/g, "&quot;");
        encodedValue = encodedValue.replace(/'/g, "&apos;");
        encodedValue = encodedValue.replace(/</g, "&lt;");
        encodedValue = encodedValue.replace(/>/g, "&gt;");
        return encodedValue;
    }

    function resetStructuredSelectionContextMenuSnapshot() {
        model.contentsView.structuredContextMenuBlockIndex = -1;
        model.contentsView.structuredContextMenuSelectionSnapshot = ({});
        model.contextMenuCoordinator.structuredContextMenuBlockIndex = -1;
        model.contextMenuCoordinator.structuredContextMenuSelectionSnapshot = ({})
    }

    function primeStructuredSelectionContextMenuSnapshot() {
        if (!model.contentsView.showStructuredDocumentFlow
                || !model.structuredDocumentFlow
                || model.structuredDocumentFlow.inlineFormatTargetState === undefined) {
            model.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        const targetState = model.structuredDocumentFlow.inlineFormatTargetState();
        const plan = model.contextMenuCoordinator.primeStructuredSelectionSnapshotPlan(
                    targetState && typeof targetState === "object" ? targetState : ({}));
        if (!plan.accepted) {
            model.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        model.contentsView.structuredContextMenuBlockIndex = Number(plan.blockIndex) || 0;
        model.contentsView.structuredContextMenuSelectionSnapshot = plan.selectionSnapshot && typeof plan.selectionSnapshot === "object"
                ? plan.selectionSnapshot
                : ({});
        model.contextMenuCoordinator.structuredContextMenuBlockIndex = model.contentsView.structuredContextMenuBlockIndex;
        model.contextMenuCoordinator.structuredContextMenuSelectionSnapshot = model.contentsView.structuredContextMenuSelectionSnapshot;
        return true;
    }

    function handleStructuredSelectionContextMenuEvent(eventName) {
        const inlineStyleTag = model.contextMenuCoordinator.inlineStyleTagForEvent(eventName === undefined || eventName === null ? "" : String(eventName));
        const plan = model.contextMenuCoordinator.handleStructuredSelectionEventPlan(
                    inlineStyleTag,
                    model.contextMenuCoordinator.structuredSelectionValid(),
                    !!(model.structuredDocumentFlow && model.structuredDocumentFlow.applyInlineFormatToBlockSelection !== undefined));
        if (!plan.applyStructuredInlineFormat) {
            if (plan.requireStructuredSelectionPrime
                    && model.primeStructuredSelectionContextMenuSnapshot()) {
                return model.handleStructuredSelectionContextMenuEvent(eventName);
            }
            return false;
        }
        const handled = !!model.structuredDocumentFlow.applyInlineFormatToBlockSelection(
                    Number(plan.blockIndex) || 0,
                    String(plan.inlineStyleTag || ""),
                    plan.selectionSnapshot && typeof plan.selectionSnapshot === "object"
                    ? plan.selectionSnapshot
                    : ({}));
        model.resetStructuredSelectionContextMenuSnapshot();
        return handled;
    }

    function openEditorSelectionContextMenu(localX, localY) {
        const editorSelectionContextMenu = model.contentsView.inputCommandSurface
                ? model.contentsView.inputCommandSurface.selectionContextMenu
                : null;
        const plan = model.contextMenuCoordinator.openSelectionContextMenuPlan(
                    model.contextMenuCoordinator.structuredSelectionValid(),
                    !!editorSelectionContextMenu,
                    Number(localX) || 0,
                    Number(localY) || 0);
        if (plan.delegateToEditorSelectionController)
            return model.editorSelectionController.openEditorSelectionContextMenu(localX, localY);
        if (plan.requireStructuredSelectionPrime
                && !model.primeStructuredSelectionContextMenuSnapshot())
            return false;
        if (!editorSelectionContextMenu)
            return false;
        if (plan.closeBeforeOpen && editorSelectionContextMenu.opened)
            editorSelectionContextMenu.close();
        editorSelectionContextMenu.openFor(
                    model.editorViewport,
                    Number(plan.openX) || 0,
                    Number(plan.openY) || 0);
        return true;
    }

    function editorContextMenuPointerTriggerAccepted(triggerKind) {
        const normalizedTrigger = triggerKind === undefined || triggerKind === null ? "" : String(triggerKind).trim().toLowerCase();
        if (normalizedTrigger === "rightclick"
                || normalizedTrigger === "right-click"
                || normalizedTrigger === "contextmenu"
                || normalizedTrigger === "context-menu") {
            return true;
        }
        if (normalizedTrigger === "longpress"
                || normalizedTrigger === "long-press"
                || normalizedTrigger === "pressandhold"
                || normalizedTrigger === "press-and-hold") {
            return model.contentsView.contextMenuLongPressEnabled;
        }
        return false;
    }

    function requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind) {
        if (!model.editorContextMenuPointerTriggerAccepted(triggerKind))
            return false;
        if (!model.ensureEditorSelectionContextMenuSnapshot())
            return false;
        Qt.callLater(function () {
            model.openEditorSelectionContextMenu(localX, localY);
        });
        return true;
    }

    function editorSelectionContextMenuSnapshotValid() {
        if (model.contentsView.showStructuredDocumentFlow)
            return model.contextMenuCoordinator.structuredSelectionValid();
        const selectionRange = model.editorSelectionController.contextMenuEditorSelectionRange();
        return selectionRange
                && Number(selectionRange.end) > Number(selectionRange.start);
    }

    function ensureEditorSelectionContextMenuSnapshot() {
        if (model.editorSelectionContextMenuSnapshotValid())
            return true;
        return model.primeEditorSelectionContextMenuSnapshot();
    }

    function primeEditorSelectionContextMenuSnapshot() {
        if (model.contentsView.showStructuredDocumentFlow)
            return model.primeStructuredSelectionContextMenuSnapshot();
        return model.editorSelectionController.primeContextMenuSelectionSnapshot();
    }
}
