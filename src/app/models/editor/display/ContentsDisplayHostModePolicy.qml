pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

QtObject {
    id: policy
    objectName: "contentsDisplayHostModePolicy"

    property bool mobileHost: false
    property bool minimapVisible: true
    property bool preferNativeInputHandling: false
    property bool showDedicatedResourceViewer: false
    property bool showFormattedTextRenderer: false
    property bool showPrintEditorLayout: false
    property bool showStructuredDocumentFlow: false

    readonly property int editorFontWeight: mobileHost ? Font.Medium : Font.Normal
    readonly property int editorHorizontalInset: mobileHost ? LV.Theme.gapNone : LV.Theme.gap16
    readonly property bool lineGeometryRefreshEnabled: !showDedicatedResourceViewer
                                                       && !showFormattedTextRenderer
                                                       && (!mobileHost || !showStructuredDocumentFlow)
    readonly property bool showEditorGutter: !mobileHost
                                             && !showDedicatedResourceViewer
                                             && !showFormattedTextRenderer
    readonly property bool showMinimapRail: minimapVisible
                                           && !showDedicatedResourceViewer
                                           && !showPrintEditorLayout
                                           && !showFormattedTextRenderer
                                           && (!mobileHost || !showStructuredDocumentFlow)
    readonly property bool minimapRefreshEnabled: showMinimapRail
    readonly property bool structuredHostGeometryActive: !mobileHost && showStructuredDocumentFlow
}
