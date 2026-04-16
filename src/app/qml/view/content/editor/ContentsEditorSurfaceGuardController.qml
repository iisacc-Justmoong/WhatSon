pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var contentEditor: null
    property var editorProjection: null
    property bool resourceDropEditorSurfaceGuardActive: false
    property int resourceDropEditorSurfaceGuardToken: 0
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

    function restoreEditorSurfaceFromPresentation() {
        if (!controller.contentEditor)
            return;

        const nextSurfaceText = String(controller.editorProjection && controller.editorProjection.logicalText !== undefined
                                           && controller.editorProjection.logicalText !== null
                                           ? controller.editorProjection.logicalText
                                           : "");
        controller.markProgrammaticEditorSurfaceSync();
        if (controller.contentEditor.setProgrammaticText !== undefined) {
            controller.contentEditor.setProgrammaticText(nextSurfaceText);
            return;
        }
        if (controller.contentEditor.text !== undefined && controller.contentEditor.text !== nextSurfaceText)
            controller.contentEditor.text = nextSurfaceText;
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
