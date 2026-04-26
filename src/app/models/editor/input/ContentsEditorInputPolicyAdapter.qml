pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: adapter
    objectName: "contentsEditorInputPolicyAdapter"

    property bool editorCompositionActive: false
    property bool editorInputFocused: false
    property bool editorTagManagementInputEnabled: true
    property bool nativeTextInputPriority: false
    property bool noteDocumentCommandSurfaceEnabled: false
    property bool structuredCompositionActive: false

    readonly property bool nativeCompositionActive: adapter.editorCompositionActive
                                                    || adapter.structuredCompositionActive
    readonly property bool nativeTextInputSessionActive: adapter.nativeCompositionActive
                                                         || adapter.editorInputFocused
    readonly property bool shortcutSurfaceEnabled: adapter.noteDocumentCommandSurfaceEnabled
                                                   && !adapter.nativeTextInputSessionActive
    readonly property bool tagManagementShortcutSurfaceEnabled: adapter.editorTagManagementInputEnabled
                                                                && adapter.noteDocumentCommandSurfaceEnabled
                                                                && !adapter.nativeCompositionActive
    readonly property bool contextMenuLongPressEnabled: adapter.nativeTextInputPriority
                                                        && !adapter.nativeTextInputSessionActive
    readonly property bool contextMenuSurfaceEnabled: adapter.editorTagManagementInputEnabled
                                                     && adapter.noteDocumentCommandSurfaceEnabled
                                                     && !adapter.nativeCompositionActive

    function normalizedText(value) {
        return value === undefined || value === null ? "" : String(value);
    }

    function programmaticTextSyncPolicy(currentText, nextText, compositionActive, focused, preferNativeInputHandling, localTextEditActive) {
        const current = adapter.normalizedText(currentText);
        const next = adapter.normalizedText(nextText);
        const nativeFocusedLocalEdit = !!preferNativeInputHandling && !!focused && !!localTextEditActive;
        if (compositionActive) {
            return {
                "apply": false,
                "defer": true
            };
        }
        if (nativeFocusedLocalEdit) {
            return {
                "apply": false,
                "defer": current !== next
            };
        }
        return {
            "apply": current !== next,
            "defer": false
        };
    }

    function shouldDeferProgrammaticTextSync(currentText, nextText, compositionActive, focused, preferNativeInputHandling, localTextEditActive) {
        const policy = adapter.programmaticTextSyncPolicy(
                    currentText,
                    nextText,
                    compositionActive,
                    focused,
                    preferNativeInputHandling,
                    localTextEditActive);
        return !!policy.defer;
    }

    function shouldApplyProgrammaticTextSync(currentText, nextText, compositionActive, focused, preferNativeInputHandling, localTextEditActive) {
        const policy = adapter.programmaticTextSyncPolicy(
                    currentText,
                    nextText,
                    compositionActive,
                    focused,
                    preferNativeInputHandling,
                    localTextEditActive);
        return !!policy.apply;
    }

    function shouldRestoreFocusForMutation(focusRequest, options) {
        if (!focusRequest || typeof focusRequest !== "object")
            return false;

        const safeOptions = options && typeof options === "object" ? options : ({});
        const compositionActive = safeOptions.compositionActive !== undefined
                ? !!safeOptions.compositionActive
                : adapter.nativeCompositionActive;
        if (compositionActive)
            return false;

        const nativePriority = safeOptions.nativeTextInputPriority !== undefined
                ? !!safeOptions.nativeTextInputPriority
                : adapter.nativeTextInputPriority;
        const focused = safeOptions.editorInputFocused !== undefined
                ? !!safeOptions.editorInputFocused
                : adapter.editorInputFocused;
        const reason = adapter.normalizedText(focusRequest.reason).trim();

        if (nativePriority && focused && reason === "text-edit")
            return false;
        return true;
    }
}
