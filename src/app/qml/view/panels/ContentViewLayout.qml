pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: contentsView

    readonly property color activeLineNumberColor: "#9DA0A8"
    property var contentViewModel: null
    readonly property int currentCursorLineNumber: contentsView.logicalLineNumberForOffset(Number(contentEditor.cursorPosition) || 0)
    readonly property color decorativeMarkerYellow: "#FFF567"
    property color displayColor: LV.Theme.panelBackground09
    property color drawerColor: LV.Theme.panelBackground11
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int editorBottomInset: 16
    property string editorBoundNoteId: ""
    readonly property real editorContentOffsetY: {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return 0;
        return Number(contentEditor.editorItem.parent.y) || 0;
    }
    readonly property var editorFlickable: contentsView.resolveEditorFlickable()
    readonly property int editorHorizontalInset: 16
    readonly property real editorLineHeight: contentsView.editorTextLineBoxHeight
    property string editorText: ""
    readonly property int editorTextLineBoxHeight: 12
    readonly property int editorTopInset: 48
    readonly property var effectiveGutterMarkers: {
        const normalizedMarkers = [];
        if (contentsView.showCurrentLineMarker) {
            normalizedMarkers.push({
                "color": contentsView.gutterMarkerCurrentColor,
                "lineSpan": 1,
                "startLine": contentsView.currentCursorLineNumber,
                "type": "current"
            });
        }
        const externalMarkers = Array.isArray(contentsView.gutterMarkers) ? contentsView.gutterMarkers : [];
        for (let i = 0; i < externalMarkers.length; ++i) {
            const normalizedMarker = contentsView.normalizeGutterMarker(externalMarkers[i]);
            if (normalizedMarker)
                normalizedMarkers.push(normalizedMarker);
        }
        return normalizedMarkers;
    }
    readonly property int frameHorizontalInset: 2
    readonly property color gutterColor: LV.Theme.panelBackground04
    readonly property int gutterCommentMarkerOffset: 2
    readonly property int gutterCommentRailLeft: 4
    readonly property int gutterCommentRailWidth: 10
    readonly property int gutterIconRailLeft: 40
    readonly property int gutterIconRailWidth: 18
    readonly property color gutterMarkerChangedColor: contentsView.decorativeMarkerYellow
    readonly property color gutterMarkerConflictColor: LV.Theme.danger
    readonly property color gutterMarkerCurrentColor: LV.Theme.primary
    property var gutterMarkers: []
    readonly property int gutterWidth: 74
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    readonly property color lineNumberColor: "#4E5157"
    readonly property int lineNumberColumnLeft: 14
    readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset
    readonly property int lineNumberColumnWidth: 26
    readonly property int lineNumberRightInset: contentsView.editorHorizontalInset
    readonly property int logicalLineCount: {
        const offsets = contentsView.logicalLineStartOffsets;
        return offsets && offsets.length !== undefined ? Math.max(1, offsets.length) : 1;
    }
    readonly property var logicalLineStartOffsets: contentsView.normalizeLogicalLineOffsets(contentsView.buildLogicalLineStartOffsets(contentsView.editorText))
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    readonly property color minimapCurrentLineColor: contentsView.activeLineNumberColor
    readonly property color minimapLineColor: contentsView.lineNumberColor
    readonly property int minimapOuterWidth: 56
    readonly property int minimapTrackInset: 8
    readonly property int minimapTrackWidth: 36
    readonly property color minimapViewportFillColor: "#149DA0A8"
    readonly property int minimapViewportMinHeight: 28
    readonly property var minimapVisualRows: contentsView.buildMinimapVisualRows(contentsView.editorText, Number(contentEditor ? contentEditor.width : 0), Number(contentEditor ? contentEditor.contentHeight : 0))
    property var noteListModel: null
    property color panelColor: LV.Theme.panelBackground07
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    property bool pendingBodySave: false
    readonly property int saveDebounceMs: 300
    readonly property string selectedNoteBodyText: noteListModel && noteListModel.currentBodyText !== undefined ? String(noteListModel.currentBodyText) : ""
    readonly property string selectedNoteId: noteListModel && noteListModel.currentNoteId !== undefined ? String(noteListModel.currentNoteId) : ""
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentEditor.focused
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property bool syncingEditorTextFromModel: false
    readonly property real textOriginY: {
        if (!contentEditor.editorItem)
            return contentsView.editorTopInset;
        return (Number(contentEditor.editorItem.y) || contentsView.editorTopInset) + contentsView.editorContentOffsetY;
    }

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested

    function buildFallbackMinimapVisualRows(textStartY) {
        const rows = [];
        for (let lineNumber = 1; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            const characterCount = contentsView.logicalLineCharacterCountAt(lineNumber - 1);
            const rowCount = Math.max(1, Math.round(contentsView.lineVisualHeight(lineNumber, 1) / Math.max(1, contentsView.editorLineHeight)));
            for (let rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                const segmentStart = Math.floor(rowIndex * characterCount / rowCount);
                const segmentEnd = Math.ceil((rowIndex + 1) * characterCount / rowCount);
                rows.push({
                    "charCount": Math.max(0, segmentEnd - segmentStart),
                    "contentHeight": contentsView.editorLineHeight,
                    "contentY": textStartY + contentsView.lineDocumentY(lineNumber) + rowIndex * contentsView.editorLineHeight,
                    "lineNumber": lineNumber
                });
            }
        }
        if (rows.length === 0) {
            rows.push({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1
            });
        }

    }
    function buildLogicalLineStartOffsets(text) {
        const value = text === undefined || text === null ? "" : String(text);
        const offsets = [0];
        for (let index = 0; index < value.length; ++index) {
            if (value.charAt(index) === "\n")
                offsets.push(index + 1);
        }

    }
    function buildMinimapVisualRows(text, editorWidth, editorContentHeight) {
        const _editorWidth = Number(editorWidth) || 0;
        const _editorContentHeight = Number(editorContentHeight) || 0;
        const value = text === undefined || text === null ? "" : String(text);
        const textStartY = contentEditor.editorItem ? Number(contentEditor.editorItem.y) || contentsView.editorTopInset : contentsView.editorTopInset;
        if (!contentEditor.editorItem || contentEditor.editorItem.positionToRectangle === undefined || _editorWidth <= 0 || _editorContentHeight <= 0)
            return contentsView.buildFallbackMinimapVisualRows(textStartY);

        const rows = [];
        if (value.length === 0) {
            rows.push({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1
            });
            return rows;
        }

        let segmentStartOffset = 0;
        let segmentStartY = Number(contentEditor.editorItem.positionToRectangle(0).y) || 0;
        for (let offset = 1; offset <= value.length; ++offset) {
            let nextY = segmentStartY + contentsView.editorLineHeight;
            if (offset < value.length) {
                const nextRect = contentEditor.editorItem.positionToRectangle(offset);
                const nextRectY = Number(nextRect.y);
                if (isFinite(nextRectY))
                    nextY = nextRectY;
            }
            const rowBreak = offset === value.length || nextY !== segmentStartY;
            if (!rowBreak)
                continue;

            let characterCount = offset - segmentStartOffset;
            if (offset > segmentStartOffset && value.charAt(offset - 1) === "\n")
                characterCount -= 1;

            rows.push({
                "charCount": Math.max(0, characterCount),
                "contentHeight": Math.max(1, nextY - segmentStartY),
                "contentY": textStartY + segmentStartY,
                "lineNumber": contentsView.logicalLineNumberForOffset(segmentStartOffset)
            });
            segmentStartOffset = offset;
            segmentStartY = nextY;
        }

        return rows.length > 0 ? rows : contentsView.buildFallbackMinimapVisualRows(textStartY);
    }
    function clampDrawerHeight(value) {
        var maxDrawer = Math.max(contentsView.minDrawerHeight, contentsView.height - contentsView.minDisplayHeight - contentsView.splitterThickness);
        return Math.max(contentsView.minDrawerHeight, Math.min(maxDrawer, value));
    }
    function clampUnit(value) {
        return Math.max(0, Math.min(1, Number(value) || 0));
    }
    function documentYForOffset(offset) {
        const safeOffset = Math.max(0, Number(offset) || 0);
        if (!contentEditor.editorItem || contentEditor.editorItem.positionToRectangle === undefined) {
            const fallbackLineNumber = contentsView.logicalLineNumberForOffset(safeOffset);
            return (fallbackLineNumber - 1) * contentsView.editorLineHeight;
        }
        const rect = contentEditor.editorItem.positionToRectangle(safeOffset);
        return Number(rect.y) || 0;
    }
    function editorViewportYForDocumentY(documentY) {
        const editorY = contentEditor.editorItem ? Number(contentEditor.editorItem.y) || contentsView.editorTopInset : contentsView.editorTopInset;
        return editorY + documentY + contentsView.editorContentOffsetY;
    }
    function firstVisibleLogicalLine() {
        let low = 1;
        let high = contentsView.logicalLineCount;
        let best = 1;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleBottomY = contentsView.lineY(middle) + contentsView.lineVisualHeight(middle, 1);
            if (middleBottomY >= 0) {
                best = middle;
                high = middle - 1;
            } else {
                low = middle + 1;
            }
        }

    }
    function flushPendingEditorText() {
        if (!contentsView.pendingBodySave)
            return true;
        const noteId = contentsView.editorBoundNoteId === undefined || contentsView.editorBoundNoteId === null ? "" : String(contentsView.editorBoundNoteId).trim();
        if (noteId.length === 0) {
            contentsView.pendingBodySave = false;
            bodySaveTimer.stop();
            return false;
        }
        const saved = contentsView.persistEditorTextForNote(noteId, contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText));
        if (saved) {
            contentsView.pendingBodySave = false;
            bodySaveTimer.stop();
        } else {
            bodySaveTimer.restart();
        }

    }
    function lineDocumentY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        return contentsView.documentYForOffset(contentsView.logicalLineStartOffsetAt(safeLineNumber - 1));
    }
    function lineVisualHeight(startLine, lineSpan) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLine) || 1));
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        const startDocumentY = contentsView.lineDocumentY(safeStartLine);
        const nextLineNumber = safeStartLine + safeLineSpan;
        let endDocumentY = 0;
        if (nextLineNumber <= contentsView.logicalLineCount) {
            endDocumentY = contentsView.lineDocumentY(nextLineNumber);
        } else {
            const contentHeight = Number(contentEditor.contentHeight) || 0;
            endDocumentY = contentHeight > 0 ? contentHeight : startDocumentY + safeLineSpan * contentsView.editorLineHeight;
        }
        return Math.max(contentsView.editorLineHeight, endDocumentY - startDocumentY);
    }
    function lineY(lineNumber) {
        return contentsView.editorViewportYForDocumentY(contentsView.lineDocumentY(lineNumber));
    }
    function logicalLineCharacterCountAt(index) {
        const offsets = contentsView.logicalLineStartOffsets;
        if (!offsets || offsets.length === undefined || offsets.length === 0)
            return 0;
        const safeIndex = Math.max(0, Math.min(offsets.length - 1, Number(index) || 0));
        const startOffset = Number(offsets[safeIndex]) || 0;
        const nextOffset = safeIndex + 1 < offsets.length ? Number(offsets[safeIndex + 1]) || startOffset : contentsView.editorText.length;
        return Math.max(0, nextOffset - startOffset - (safeIndex + 1 < offsets.length ? 1 : 0));
    }
    function logicalLineNumberForDocumentY(documentY) {
        const offsets = contentsView.logicalLineStartOffsets;
        if (!offsets || offsets.length === undefined || offsets.length === 0)
            return 1;
        const safeDocumentY = Math.max(0, Number(documentY) || 0);
        let low = 0;
        let high = offsets.length - 1;
        let best = 0;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleY = contentsView.documentYForOffset(offsets[middle]);
            if (middleY <= safeDocumentY) {
                best = middle;
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        return best + 1;
    }
    function logicalLineNumberForOffset(offset) {
        const offsets = contentsView.logicalLineStartOffsets;
        if (!offsets || offsets.length === undefined || offsets.length === 0)
            return 1;
        const safeOffset = Math.max(0, Number(offset) || 0);
        let low = 0;
        let high = offsets.length - 1;
        let best = 0;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleOffset = Number(offsets[middle]) || 0;
            if (middleOffset <= safeOffset) {
                best = middle;
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        return best + 1;
    }
    function logicalLineStartOffsetAt(index) {
        const offsets = contentsView.logicalLineStartOffsets;
        if (!offsets || offsets.length === undefined || offsets.length === 0)
            return 0;
        const safeIndex = Math.max(0, Math.min(offsets.length - 1, Number(index) || 0));
        return Number(offsets[safeIndex]) || 0;
    }
    function markerColorForType(markerType) {
        const normalizedType = markerType === undefined || markerType === null ? "" : String(markerType).toLowerCase();
        if (normalizedType === "conflict")
            return contentsView.gutterMarkerConflictColor;
        if (normalizedType === "changed")
            return contentsView.gutterMarkerChangedColor;

    }
    function markerY(markerSpec) {
        if (!markerSpec)
            return contentsView.editorTopInset;
        const startLine = Math.max(1, Number(markerSpec.startLine) || 1);
        return contentsView.lineY(startLine);
    }
    function minimapBarWidth(characterCount) {
        const safeCount = Math.max(0, Number(characterCount) || 0);
        const maxWidth = Math.max(6, minimapTrack.width - 1);
        if (safeCount <= 0)
            return Math.max(2, maxWidth * 0.08);
        const widthRatio = contentsView.clampUnit(0.08 + Math.log(safeCount + 1) / Math.log(160));
        return Math.max(4, maxWidth * widthRatio);
    }
    function minimapContentHeight() {
        const flickable = contentsView.editorFlickable;
        if (flickable)
            return Math.max(1, Number(flickable.contentHeight) || 0);
        return Math.max(1, Number(contentEditor.contentHeight) || 0);
    }
    function minimapContentYForLine(lineNumber) {
        const textStartY = contentEditor.editorItem ? Number(contentEditor.editorItem.y) || contentsView.editorTopInset : contentsView.editorTopInset;
        return textStartY + contentsView.lineDocumentY(lineNumber);
    }
    function minimapCurrentLineHeight() {
        const visualRow = contentsView.minimapCurrentVisualRow();
        return Math.max(1, Math.min(2, contentsView.minimapTrackHeightForContentHeight(visualRow.contentHeight)));
    }
    function minimapCurrentLineWidth() {
        const visualRow = contentsView.minimapCurrentVisualRow();
        return contentsView.minimapBarWidth(visualRow.charCount);
    }
    function minimapCurrentLineY() {
        const visualRow = contentsView.minimapCurrentVisualRow();
        return contentsView.minimapTrackYForContentY(visualRow.contentY);
    }
    function minimapCurrentVisualRow() {
        const rows = Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : [];
        const textStartY = contentEditor.editorItem ? Number(contentEditor.editorItem.y) || contentsView.editorTopInset : contentsView.editorTopInset;
        const cursorRectY = contentEditor.cursorRectangle ? Number(contentEditor.cursorRectangle.y) || 0 : contentsView.documentYForOffset(Number(contentEditor.cursorPosition) || 0);
        const cursorContentY = textStartY + cursorRectY;
        for (let index = 0; index < rows.length; ++index) {
            const row = rows[index];
            const rowStart = Number(row.contentY) || 0;
            const rowEnd = rowStart + Math.max(1, Number(row.contentHeight) || contentsView.editorLineHeight);
            if (cursorContentY >= rowStart && cursorContentY < rowEnd)
                return row;
        }
        return rows.length > 0 ? rows[0] : ({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1
            });
    }
    function minimapLineY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        return contentsView.minimapTrackYForContentY(contentsView.minimapContentYForLine(safeLineNumber));
    }
    function minimapTrackHeightForContentHeight(segmentHeight) {
        const safeSegmentHeight = Math.max(0, Number(segmentHeight) || 0);
        const contentHeight = contentsView.minimapContentHeight();
        if (contentHeight <= 0)
            return 0;
        return Math.max(1, (safeSegmentHeight / contentHeight) * minimapTrack.height);
    }
    function minimapTrackYForContentY(contentY) {
        const contentHeight = contentsView.minimapContentHeight();
        if (contentHeight <= 0)
            return 0;
        const safeContentY = Math.max(0, Math.min(contentHeight, Number(contentY) || 0));
        return (safeContentY / contentHeight) * minimapTrack.height;
    }
    function minimapViewportHeight() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return minimapTrack.height;
        const contentHeight = Math.max(1, Number(flickable.contentHeight) || 0);
        const viewportHeight = Math.max(0, Number(flickable.height) || 0);
        if (contentHeight <= viewportHeight)
            return minimapTrack.height;
        const proportionalHeight = contentsView.minimapTrackHeightForContentHeight(viewportHeight);
        return Math.min(minimapTrack.height, Math.max(contentsView.minimapViewportMinHeight, proportionalHeight));
    }
    function minimapViewportY() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return 0;
        const contentHeight = Math.max(1, Number(flickable.contentHeight) || 0);
        const viewportHeight = Math.max(0, Number(flickable.height) || 0);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        if (maxContentY <= 0)
            return 0;
        const contentY = Math.max(0, Math.min(maxContentY, Number(flickable.contentY) || 0));
        const maxTrackY = Math.max(0, minimapTrack.height - contentsView.minimapViewportHeight());
        return maxTrackY * (contentY / maxContentY);
    }
    function minimapVisualRowPaintHeight(rowSpec) {
        const sourceHeight = rowSpec && rowSpec.contentHeight !== undefined ? Number(rowSpec.contentHeight) || contentsView.editorLineHeight : contentsView.editorLineHeight;
        const trackSlotHeight = contentsView.minimapTrackHeightForContentHeight(sourceHeight);
        return Math.max(0.8, Math.min(1.5, trackSlotHeight * 0.58));
    }
    function minimapVisualRowPaintY(rowSpec) {
        const rowY = contentsView.minimapTrackYForContentY(rowSpec && rowSpec.contentY !== undefined ? rowSpec.contentY : 0);
        const sourceHeight = rowSpec && rowSpec.contentHeight !== undefined ? Number(rowSpec.contentHeight) || contentsView.editorLineHeight : contentsView.editorLineHeight;
        const trackSlotHeight = contentsView.minimapTrackHeightForContentHeight(sourceHeight);
        return rowY + Math.max(0, (trackSlotHeight - contentsView.minimapVisualRowPaintHeight(rowSpec)) / 2);
    }
    function normalizeGutterMarker(markerSpec) {
        if (!markerSpec)
            return null;
        const rawStartLine = markerSpec.startLine !== undefined ? markerSpec.startLine : markerSpec.line;
        const startLine = Math.max(1, Number(rawStartLine) || 1);
        const hasExplicitSpan = markerSpec.lineSpan !== undefined && markerSpec.lineSpan !== null;
        const explicitSpan = hasExplicitSpan ? Math.max(1, Number(markerSpec.lineSpan) || 1) : 0;
        const endLine = Math.max(startLine, Number(markerSpec.endLine) || startLine);
        const lineSpan = explicitSpan > 0 ? explicitSpan : Math.max(1, endLine - startLine + 1);
        const markerType = markerSpec.type !== undefined ? String(markerSpec.type).toLowerCase() : "";
        if (markerType !== "changed" && markerType !== "conflict" && markerType !== "current")
            return null;
        return {
            "color": contentsView.markerColorForType(markerType),
            "lineSpan": lineSpan,
            "startLine": startLine,
            "type": markerType
        };
    }
    function normalizeLogicalLineOffsets(offsets) {
        if (!offsets || offsets.length === undefined || offsets.length === 0)
            return [0];

    }
    function persistEditorTextForNote(noteId, text) {
        if (!contentViewModel)
            return false;
        if (contentViewModel.saveBodyTextForNote !== undefined)
            return Boolean(contentViewModel.saveBodyTextForNote(noteId, text));
        if (contentViewModel.saveCurrentBodyText !== undefined)
            return Boolean(contentViewModel.saveCurrentBodyText(text));
        return false;
    }
    function releaseEditorSyncGuard() {
        Qt.callLater(function () {
            contentsView.syncingEditorTextFromModel = false;
        });
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function resolveEditorFlickable() {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return null;
        const candidate = contentEditor.editorItem.parent.parent;
        if (!candidate || candidate.contentY === undefined || candidate.contentHeight === undefined || candidate.height === undefined)
            return null;

    }
    function scheduleEditorPersistence() {
        contentsView.pendingBodySave = true;
        bodySaveTimer.restart();
    }
    function scrollEditorViewportToMinimapPosition(localY) {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return;
        const contentHeight = Math.max(1, Number(flickable.contentHeight) || 0);
        const viewportHeight = Math.max(0, Number(flickable.height) || 0);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        if (maxContentY <= 0) {
            flickable.contentY = 0;
            return;
        }
        const trackRatio = contentsView.clampUnit((Number(localY) || 0) / Math.max(1, minimapTrack.height));
        const documentY = contentHeight * trackRatio;
        const nextContentY = Math.max(0, Math.min(maxContentY, documentY - viewportHeight / 2));
        flickable.contentY = nextContentY;
    }
    function syncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        bodySaveTimer.stop();
        contentsView.pendingBodySave = false;
        contentsView.editorBoundNoteId = nextNoteId;
        contentsView.syncingEditorTextFromModel = true;
        if (contentsView.editorText !== nextText)
            contentsView.editorText = nextText;
        contentsView.releaseEditorSyncGuard();
    }
    function visibleLineNumbers() {
        const visibleLines = [];
        const firstVisibleLine = contentsView.firstVisibleLogicalLine();
        for (let lineNumber = firstVisibleLine; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            const lineY = contentsView.lineY(lineNumber);
            if (lineY > lineNumberViewport.height)
                break;
            if (lineY + contentsView.editorLineHeight < 0)
                continue;
            visibleLines.push(lineNumber);
        }
        if (visibleLines.length === 0)
            visibleLines.push(firstVisibleLine);

    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    Component.onCompleted: contentsView.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText)
    Component.onDestruction: contentsView.flushPendingEditorText()
    onEditorTextChanged: minimapCanvas.requestPaint()
    onSelectedNoteBodyTextChanged: {
        contentsView.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);
    }
    onSelectedNoteIdChanged: {
        if (contentsView.pendingBodySave && contentsView.editorBoundNoteId !== contentsView.selectedNoteId)
            contentsView.flushPendingEditorText();
    }

    Timer {
        id: bodySaveTimer

        interval: contentsView.saveDebounceMs
        repeat: false

        onTriggered: contentsView.flushPendingEditorText()
    }
    Rectangle {
        anchors.fill: parent
        color: contentsView.panelColor
    }
    LV.VStack {
        id: drawerView

        anchors.fill: parent
        spacing: LV.Theme.gapNone

        Rectangle {
            id: contentsDisplayView

            Layout.fillHeight: true
            Layout.fillWidth: true
            color: contentsView.displayColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: contentsView.frameHorizontalInset
                anchors.rightMargin: contentsView.frameHorizontalInset
                spacing: 0

                Rectangle {
                    id: lineNumberGutter

                    Layout.fillHeight: true
                    Layout.preferredWidth: contentsView.gutterWidth
                    color: contentsView.gutterColor

                    Item {
                        id: lineNumberViewport

                        anchors.fill: parent
                        clip: true

                        Item {
                            height: parent.height
                            width: contentsView.gutterIconRailWidth
                            x: contentsView.gutterIconRailLeft
                        }
                        Repeater {
                            model: contentsView.effectiveGutterMarkers

                            delegate: Rectangle {
                                readonly property var markerSpec: modelData || ({
                                        "color": contentsView.gutterMarkerCurrentColor,
                                        "lineSpan": 1,
                                        "startLine": 1
                                    })
                                required property var modelData

                                color: markerSpec.color
                                height: Math.max(20, contentsView.lineVisualHeight(markerSpec.startLine, markerSpec.lineSpan))
                                radius: width / 2
                                width: 4
                                x: contentsView.gutterCommentRailLeft + contentsView.gutterCommentMarkerOffset
                                y: contentsView.markerY(markerSpec)
                            }
                        }
                        Repeater {
                            model: contentsView.visibleLineNumbers()

                            delegate: LV.Label {
                                required property int modelData

                                color: modelData === contentsView.currentCursorLineNumber ? contentsView.activeLineNumberColor : contentsView.lineNumberColor
                                font.family: LV.Theme.fontBody
                                font.letterSpacing: 0
                                font.pixelSize: 11
                                font.weight: modelData === contentsView.currentCursorLineNumber ? Font.Medium : Font.Normal
                                height: contentsView.editorLineHeight
                                horizontalAlignment: Text.AlignRight
                                style: caption
                                text: String(modelData)
                                verticalAlignment: Text.AlignVCenter
                                width: contentsView.lineNumberColumnTextWidth
                                x: contentsView.lineNumberColumnLeft
                                y: contentsView.lineY(modelData)
                            }
                        }
                    }
                }
                Item {
                    id: editorViewport

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: contentsView.minEditorHeight

                    LV.TextEditor {
                        id: contentEditor

                        anchors.fill: parent
                        autoFocusOnPress: true
                        backgroundColor: contentsView.displayColor
                        backgroundColorDisabled: contentsView.displayColor
                        backgroundColorFocused: contentsView.displayColor
                        backgroundColorHover: contentsView.displayColor
                        backgroundColorPressed: contentsView.displayColor
                        centeredTextHeight: contentsView.editorTextLineBoxHeight
                        cornerRadius: 0
                        editorHeight: editorViewport.height
                        enforceModeDefaults: false
                        fieldMinHeight: Math.max(contentsView.minEditorHeight, editorViewport.height)
                        fontFamily: LV.Theme.fontBody
                        fontLetterSpacing: 0
                        fontPixelSize: 12
                        fontWeight: Font.Medium
                        insetHorizontal: contentsView.editorHorizontalInset
                        insetVertical: contentsView.editorBottomInset
                        placeholderText: ""
                        selectByMouse: true
                        selectedTextColor: LV.Theme.textPrimary
                        selectionColor: LV.Theme.accentBlueMuted
                        shapeStyle: shapeRoundRect
                        showRenderedOutput: false
                        showScrollBar: false
                        text: contentsView.editorText
                        textColor: LV.Theme.textSecondary
                        textFormat: TextEdit.PlainText
                        wrapMode: TextEdit.Wrap

                        onFocusedChanged: {
                            if (!focused)
                                contentsView.flushPendingEditorText();
                        }
                        onTextEdited: function (text) {
                            if (contentsView.editorText !== text)
                                contentsView.editorText = text;
                            if (contentsView.syncingEditorTextFromModel)
                                return;
                            contentsView.scheduleEditorPersistence();
                            contentsView.editorTextEdited(text);
                        }
                    }
                    Binding {
                        "y"
                        target: contentEditor.editorItem
                        value: contentsView.editorTopInset
                    }
                    LV.Label {
                        anchors.left: parent.left
                        anchors.leftMargin: contentsView.editorHorizontalInset
                        anchors.top: parent.top
                        anchors.topMargin: contentsView.editorTopInset
                        color: LV.Theme.textTertiary
                        style: body
                        text: contentsView.hasSelectedNote ? "Start typing here" : "Select a note to view its body text"
                        visible: contentEditor.empty
                    }
                }
                Item {
                    id: minimapRail

                    Layout.fillHeight: true
                    Layout.preferredWidth: contentsView.minimapOuterWidth

                    Item {
                        id: minimapTrack

                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: contentsView.minimapTrackInset
                        anchors.top: parent.top
                        anchors.topMargin: 8
                        width: contentsView.minimapTrackWidth

                        Canvas {
                            id: minimapCanvas

                            anchors.fill: parent
                            renderTarget: Canvas.Image

                            onHeightChanged: requestPaint()
                            onPaint: {
                                const context = getContext("2d");
                                context.clearRect(0, 0, width, height);

                                const rows = Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : [];
                                for (let rowIndex = 0; rowIndex < rows.length; ++rowIndex) {
                                    const row = rows[rowIndex];
                                    const characterCount = Number(row.charCount) || 0;
                                    const barY = contentsView.minimapVisualRowPaintY(row);
                                    const barHeight = contentsView.minimapVisualRowPaintHeight(row);
                                    if (barY > height || barY + barHeight < 0)
                                        continue;
                                    context.fillStyle = contentsView.minimapLineColor;
                                    context.globalAlpha = characterCount > 0 ? 0.48 : 0.12;
                                    context.fillRect(0, barY, contentsView.minimapBarWidth(characterCount), barHeight);
                                }
                                context.globalAlpha = 1;
                            }
                            onWidthChanged: requestPaint()
                        }
                        Rectangle {
                            readonly property bool scrollable: contentsView.editorFlickable && contentsView.minimapContentHeight() > (Number(contentsView.editorFlickable.height) || 0)

                            anchors.left: parent.left
                            anchors.right: parent.right
                            border.width: 0
                            color: contentsView.minimapViewportFillColor
                            height: contentsView.minimapViewportHeight()
                            radius: 3
                            visible: scrollable
                            y: contentsView.minimapViewportY()
                        }
                        Rectangle {
                            color: contentsView.minimapCurrentLineColor
                            height: Math.max(1, contentsView.minimapCurrentLineHeight())
                            opacity: 0.8
                            radius: 1
                            width: contentsView.minimapCurrentLineWidth()
                            x: 0
                            y: contentsView.minimapCurrentLineY()
                        }
                        MouseArea {
                            acceptedButtons: Qt.LeftButton
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true

                            onPositionChanged: function (mouse) {
                                if (!(pressedButtons & Qt.LeftButton))
                                    return;
                                contentsView.scrollEditorViewportToMinimapPosition(mouse.y);
                            }
                            onPressed: function (mouse) {
                                contentsView.scrollEditorViewportToMinimapPosition(mouse.y);
                            }
                        }
                        LV.WheelScrollGuard {
                            anchors.fill: parent
                            consumeInside: true
                            targetFlickable: contentsView.editorFlickable
                        }
                    }
                }
            }
        }
        Rectangle {
            id: drawerSplitter

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.splitterThickness
            color: contentsView.splitterColor

            MouseArea {
                id: drawerSplitterMouse

                property int dragStartDrawerHeight: contentsView.drawerHeight
                property real dragStartGlobalY: 0

                acceptedButtons: Qt.LeftButton
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                cursorShape: Qt.SizeVerCursor
                height: contentsView.splitterHandleThickness
                preventStealing: true

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaY = movePoint.y - dragStartGlobalY;
                    var nextDrawerHeight = contentsView.clampDrawerHeight(dragStartDrawerHeight - deltaY);
                    if (nextDrawerHeight !== contentsView.drawerHeight)
                        contentsView.drawerHeightDragRequested(nextDrawerHeight);
                }
                onPressed: function (mouse) {
                    var pressPoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalY = pressPoint.y;
                    dragStartDrawerHeight = contentsView.drawerHeight;
                }
            }
        }
        Rectangle {
            id: drawer

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.clampDrawerHeight(contentsView.drawerHeight)
            color: contentsView.drawerColor
        }
    }
}
