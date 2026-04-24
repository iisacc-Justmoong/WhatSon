pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var contentEditor: null
    property var editorProjection: null
    property bool resourceDropEditorSurfaceGuardActive: false
    property int resourceDropEditorSurfaceGuardToken: 0
    property bool pendingEditorSurfaceRestore: false
    property int programmaticEditorSurfaceSyncDepth: 0
    readonly property bool programmaticEditorSurfaceSyncActive: controller.programmaticEditorSurfaceSyncDepth > 0

    function activateResourceDropEditorSurfaceGuard() {
        controller.resourceDropEditorSurfaceGuardToken += 1;
        if (!controller.resourceDropEditorSurfaceGuardActive)
            controller.resourceDropEditorSurfaceGuardActive = true;
    }

    function markProgrammaticEditorSurfaceSync() {
        controller.programmaticEditorSurfaceSyncDepth += 1;
        Qt.callLater(function () {
            Qt.callLater(function () {
                controller.programmaticEditorSurfaceSyncDepth = Math.max(
                            0,
                            (Number(controller.programmaticEditorSurfaceSyncDepth) || 0) - 1);
            });
        });
    }

    function nativeCompositionActive() {
        if (!controller.contentEditor)
            return false;
        if (controller.contentEditor.nativeCompositionActive !== undefined)
            return !!controller.contentEditor.nativeCompositionActive();
        const activePreeditText = controller.contentEditor.preeditText !== undefined
                ? String(controller.contentEditor.preeditText === undefined || controller.contentEditor.preeditText === null
                             ? ""
                             : controller.contentEditor.preeditText)
                : "";
        return (controller.contentEditor.inputMethodComposing !== undefined
                && controller.contentEditor.inputMethodComposing)
                || activePreeditText.length > 0;
    }

    function presentationSurfaceText() {
        return String(controller.editorProjection && controller.editorProjection.logicalText !== undefined
                                           && controller.editorProjection.logicalText !== null
                                           ? controller.editorProjection.logicalText
                                           : "");
    }

    function shouldRejectEditorSurfaceRestore(nextSurfaceText) {
        if (!controller.contentEditor)
            return true;
        if (controller.contentEditor.shouldRejectFocusedProgrammaticTextSync !== undefined
                && controller.contentEditor.shouldRejectFocusedProgrammaticTextSync(nextSurfaceText)) {
            return true;
        }
        return false;
    }

    function restoreEditorSurfaceFromPresentation() {
        if (!controller.contentEditor) {
            controller.pendingEditorSurfaceRestore = false;
            return false;
        }

        const nextSurfaceText = controller.presentationSurfaceText();
        if (controller.nativeCompositionActive()) {
            controller.pendingEditorSurfaceRestore = true;
            return false;
        }
        if (controller.shouldRejectEditorSurfaceRestore(nextSurfaceText)) {
            controller.pendingEditorSurfaceRestore = false;
            return false;
        }
        controller.pendingEditorSurfaceRestore = false;
        controller.markProgrammaticEditorSurfaceSync();
        if (controller.contentEditor.setProgrammaticText !== undefined) {
            controller.contentEditor.setProgrammaticText(nextSurfaceText);
            return true;
        }
        if (controller.contentEditor.text !== undefined && controller.contentEditor.text !== nextSurfaceText) {
            controller.contentEditor.text = nextSurfaceText;
            return true;
        }
        return false;
    }

    function restorePendingEditorSurfaceFromPresentationIfInputSettled() {
        if (!controller.pendingEditorSurfaceRestore || controller.nativeCompositionActive())
            return false;
        return controller.restoreEditorSurfaceFromPresentation();
    }

    function releaseResourceDropEditorSurfaceGuard(restoreSurface) {
        const scheduledToken = Math.max(0, Number(controller.resourceDropEditorSurfaceGuardToken) || 0);
        if (!restoreSurface) {
            controller.resourceDropEditorSurfaceGuardActive = false;
            return;
        }

        Qt.callLater(function () {
            Qt.callLater(function () {
                if (controller.resourceDropEditorSurfaceGuardToken !== scheduledToken)
                    return;
                controller.restoreEditorSurfaceFromPresentation();
                Qt.callLater(function () {
                    if (controller.resourceDropEditorSurfaceGuardToken !== scheduledToken)
                        return;
                    controller.resourceDropEditorSurfaceGuardActive = false;
                });
            });
        });
    }
}
