pragma ComponentBehavior: Bound

import QtQuick
import "../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

QtObject {
    id: controller

    property var contentsAgendaBackend: null
    property var contentsView: null
    property var editOperationCoordinator: null
    property var editorInputPolicyAdapter: null
    property var editorSelectionController: null
    property var editorSession: null
    property var editorTypingController: null
    property var eventPump: null
    property var presentationRefreshController: null
    property var resourceImportController: null
    property var structuredDocumentFlow: null

    function queueStructuredInlineFormatWrap(tagName) {
        if (!controller.contentsView.showStructuredDocumentFlow
                || !controller.structuredDocumentFlow)
            return false;
        if (controller.structuredDocumentFlow.inlineFormatTargetState === undefined
                || controller.structuredDocumentFlow.applyInlineFormatToBlockSelection === undefined) {
            if (controller.structuredDocumentFlow.applyInlineFormatToActiveSelection !== undefined)
                return controller.structuredDocumentFlow.applyInlineFormatToActiveSelection(tagName);
            return false;
        }
        const targetState = controller.structuredDocumentFlow.inlineFormatTargetState();
        if (!targetState || !targetState.valid)
            return false;
        const normalizedNoteId = controller.contentsView.selectedNoteId === undefined
                || controller.contentsView.selectedNoteId === null
                ? ""
                : String(controller.contentsView.selectedNoteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        const blockIndex = Math.max(0, Math.floor(Number(targetState.blockIndex) || 0));
        const selectionSnapshot = targetState.selectionSnapshot && typeof targetState.selectionSnapshot === "object"
                ? ({
                       "cursorPosition": Number(targetState.selectionSnapshot.cursorPosition),
                       "selectedText": targetState.selectionSnapshot.selectedText === undefined
                                       || targetState.selectionSnapshot.selectedText === null
                                       ? ""
                                       : String(targetState.selectionSnapshot.selectedText),
                       "selectionEnd": Number(targetState.selectionSnapshot.selectionEnd),
                       "selectionStart": Number(targetState.selectionSnapshot.selectionStart)
                   })
                : ({});
        if (normalizedNoteId !== controller.contentsView.selectedNoteId)
            return false;
        return controller.structuredDocumentFlow.applyInlineFormatToBlockSelection(
                    blockIndex,
                    tagName,
                    selectionSnapshot);
    }

    function queueInlineFormatWrap(tagName) {
        if (controller.queueStructuredInlineFormatWrap(tagName))
            return true;
        return controller.editorSelectionController.queueInlineFormatWrap(tagName);
    }

    function queueAgendaShortcutInsertion() {
        if (controller.contentsView.showStructuredDocumentFlow
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            if (controller.structuredDocumentFlow.insertStructuredShortcutAtActivePosition("agenda"))
                return true;
        }
        return controller.editorTypingController.queueAgendaShortcutInsertion();
    }

    function queueCalloutShortcutInsertion() {
        if (controller.contentsView.showStructuredDocumentFlow
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            if (controller.structuredDocumentFlow.insertStructuredShortcutAtActivePosition("callout"))
                return true;
        }
        return controller.editorTypingController.queueCalloutShortcutInsertion();
    }

    function queueBreakShortcutInsertion() {
        if (controller.contentsView.showStructuredDocumentFlow
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            if (controller.structuredDocumentFlow.insertStructuredShortcutAtActivePosition("break"))
                return true;
        }
        return controller.editorTypingController.queueBreakShortcutInsertion();
    }

    function requestStructuredDocumentEndEdit() {
        if (controller.contentsView.showStructuredDocumentFlow
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.requestDocumentEndEdit !== undefined) {
            return controller.structuredDocumentFlow.requestDocumentEndEdit();
        }
        return false;
    }

    function commitRawEditorTextMutation(nextSourceText) {
        if (!controller.editorSession
                || controller.editorSession.commitRawEditorTextMutation === undefined) {
            return false;
        }
        return !!controller.editorSession.commitRawEditorTextMutation(nextSourceText);
    }

    function committedEditorText(fallbackText) {
        if (controller.editorSession && controller.editorSession.editorText !== undefined
                && controller.editorSession.editorText !== null) {
            return String(controller.editorSession.editorText);
        }
        return fallbackText === undefined || fallbackText === null ? "" : String(fallbackText);
    }

    function focusStructuredBlockSourceOffset(sourceOffset) {
        const focusPlan = controller.editOperationCoordinator.focusStructuredSourceOffsetPlan(
                    controller.contentsView.showStructuredDocumentFlow,
                    controller.structuredDocumentFlow && controller.structuredDocumentFlow.requestFocus !== undefined,
                    Math.floor(Number(sourceOffset) || 0));
        if (focusPlan.handled) {
            controller.structuredDocumentFlow.requestFocus({
                                                             "sourceOffset": Number(focusPlan.targetOffset) || 0
                                                         });
            return true;
        }
        return false;
    }

    function applyDocumentSourceMutation(nextSourceText, focusRequest) {
        const currentSourceText = controller.contentsView.editorText === undefined
                || controller.contentsView.editorText === null
                ? ""
                : String(controller.contentsView.editorText);
        const normalizedNextSourceText = nextSourceText === undefined || nextSourceText === null
                ? ""
                : String(nextSourceText);
        EditorTrace.trace(
                    "displayView",
                    "applyDocumentSourceMutation",
                    "selectedNoteId=" + controller.contentsView.selectedNoteId
                    + " focusRequest={" + EditorTrace.describeFocusRequest(focusRequest) + "} "
                    + EditorTrace.describeText(normalizedNextSourceText),
                    controller.contentsView)
        if (normalizedNextSourceText === currentSourceText)
            return false;
        if (controller.resourceImportController.resourceTagLossDetected(currentSourceText, normalizedNextSourceText)) {
            controller.resourceImportController.restoreEditorSurfaceFromPresentation();
            return false;
        }
        if (!controller.commitRawEditorTextMutation(normalizedNextSourceText))
            return false;
        const committedText = controller.committedEditorText(normalizedNextSourceText);
        controller.presentationRefreshController.clearPendingWhileFocused();
        if (!controller.contentsView.showStructuredDocumentFlow
                && controller.contentsView.commitDocumentPresentationRefresh !== undefined) {
            controller.contentsView.commitDocumentPresentationRefresh();
        } else {
            controller.eventPump.stopDocumentPresentationRefreshTimer();
        }
        if (focusRequest
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.requestFocus !== undefined
                && controller.editorInputPolicyAdapter.shouldRestoreFocusForMutation(
                    focusRequest,
                    {
                        "compositionActive": controller.editorInputPolicyAdapter.nativeCompositionActive,
                        "editorInputFocused": controller.contentsView.editorInputFocused,
                        "nativeTextInputPriority": controller.contentsView.nativeTextInputPriority
                    })) {
            const requestedFocus = focusRequest && typeof focusRequest === "object" ? focusRequest : ({});
            Qt.callLater(function () {
                controller.structuredDocumentFlow.requestFocus(requestedFocus);
            });
        }
        controller.contentsView.editorTextEdited(committedText);
        return true;
    }

    function setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked) {
        const currentSourceText = controller.contentsView.editorText === undefined
                || controller.contentsView.editorText === null
                ? ""
                : String(controller.contentsView.editorText);
        if (!controller.contentsAgendaBackend
                || controller.contentsAgendaBackend.rewriteTaskDoneAttribute === undefined) {
            return false;
        }
        const nextSourceText = String(controller.contentsAgendaBackend.rewriteTaskDoneAttribute(
                                          currentSourceText,
                                          Math.floor(Number(taskOpenTagStart) || 0),
                                          Math.floor(Number(taskOpenTagEnd) || 0),
                                          !!checked));
        return controller.applyDocumentSourceMutation(
                    nextSourceText,
                    {
                        "taskOpenTagStart": Math.floor(Number(taskOpenTagStart) || -1)
                    });
    }

    function persistEditorTextImmediately(nextText) {
        return controller.editorSelectionController.persistEditorTextImmediately(nextText);
    }

    function selectedEditorRange() {
        return controller.editorSelectionController.selectedEditorRange();
    }

    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        return controller.editorSelectionController.wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange);
    }
}
