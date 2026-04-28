pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: state

    property var surfacePolicy: null
    property var editorInputPolicyAdapter: null

    readonly property bool preferNativeInputHandling: !!(surfacePolicy && surfacePolicy.nativeInputPriority)
    readonly property bool nativeTextInputPriority: !!(surfacePolicy && surfacePolicy.nativeInputPriority)
    readonly property bool documentPresentationProjectionEnabled: !!(surfacePolicy && surfacePolicy.documentPresentationProjectionEnabled)
    readonly property bool inlineHtmlImageRenderingEnabled: !!(surfacePolicy && surfacePolicy.inlineHtmlImageRenderingEnabled)
    readonly property bool resourceBlocksRenderedInlineByHtmlProjection: !!(surfacePolicy && surfacePolicy.resourceBlocksRenderedInlineByHtmlProjection)
    readonly property bool showDedicatedResourceViewer: !!(surfacePolicy && surfacePolicy.dedicatedResourceViewerVisible)
    readonly property bool showFormattedTextRenderer: !!(surfacePolicy && surfacePolicy.formattedTextRendererVisible)
    readonly property bool showStructuredDocumentFlow: !!(surfacePolicy && surfacePolicy.structuredDocumentFlowVisible)
    readonly property string activeSurfaceKind: surfacePolicy && surfacePolicy.activeSurfaceKind !== undefined && surfacePolicy.activeSurfaceKind !== null ? String(surfacePolicy.activeSurfaceKind) : ""
    readonly property bool structuredDocumentFlowRequested: !!(surfacePolicy && surfacePolicy.structuredDocumentSurfaceRequested)

    readonly property bool editorCustomTextInputEnabled: false
    readonly property bool editorTagManagementInputEnabled: true
    readonly property bool contextMenuLongPressEnabled: !!(editorInputPolicyAdapter && editorInputPolicyAdapter.contextMenuLongPressEnabled)
    readonly property bool noteDocumentShortcutSurfaceEnabled: !!(editorInputPolicyAdapter && editorInputPolicyAdapter.shortcutSurfaceEnabled)
    readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: !!(editorInputPolicyAdapter && editorInputPolicyAdapter.tagManagementShortcutSurfaceEnabled)
    readonly property bool noteDocumentContextMenuSurfaceEnabled: !!(editorInputPolicyAdapter && editorInputPolicyAdapter.contextMenuSurfaceEnabled)
}
