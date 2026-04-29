pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: state

    property var gutterMarkers: []
    property int gutterRefreshPassesRemaining: 0
    property int gutterRefreshRevision: 0

    property int liveLogicalLineCount: 1
    property var logicalLineDocumentYCache: []
    property int logicalLineDocumentYCacheLineCount: 0
    property int logicalLineDocumentYCacheRevision: -1
    property var logicalLineGutterDocumentYCache: []
    property int logicalLineGutterDocumentYCacheLineCount: 0
    property int logicalLineGutterDocumentYCacheRevision: -1
    property string structuredGutterGeometrySignature: ""
    property var liveLogicalLineStartOffsets: [0]
    property int liveLogicalTextLength: 0
    property var visibleGutterLineEntries: [
        {
            "lineNumber": 1,
            "y": 0
        }
    ]
    property string pendingNoteEntryGutterRefreshNoteId: ""

    property real minimapResolvedCurrentLineHeight: 1
    property real minimapResolvedCurrentLineWidth: 0
    property real minimapResolvedCurrentLineY: 0
    property real minimapResolvedSilhouetteHeight: 1
    property real minimapResolvedTrackHeight: 1
    property real minimapResolvedViewportHeight: 0
    property real minimapResolvedViewportY: 0
    property var minimapLineGroups: []
    property string minimapLineGroupsNoteId: ""
    property bool minimapScrollable: false
    property bool minimapVisible: true
    property var minimapVisualRows: []

    property bool cursorDrivenUiRefreshQueued: false
    property bool typingViewportCorrectionQueued: false
    property bool typingViewportForceCorrectionRequested: false
    property bool viewportGutterRefreshQueued: false
    property bool minimapSnapshotRefreshQueued: false
}
