#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContentsView_keepsOnlyGutterTextEditorMinimapViews()
{
    const QDir contentsDir(QStringLiteral("src/app/qml/view/contents"));
    const QString gutterSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Gutter.qml"));
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));
    const QString minimapSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Minimap.qml"));
    const QString contentViewLayoutSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(contentsDir.exists());
    const QStringList contentQmlFiles = contentsDir.entryList(QStringList{QStringLiteral("*.qml")}, QDir::Files, QDir::Name);
    QCOMPARE(contentQmlFiles, QStringList({QStringLiteral("Gutter.qml"), QStringLiteral("Minimap.qml"), QStringLiteral("TextEditor.qml")}));
    QVERIFY(!QDir(contentsDir.filePath(QStringLiteral("editor"))).exists());

    QVERIFY(!gutterSource.isEmpty());
    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!minimapSource.isEmpty());
    QVERIFY(!contentViewLayoutSource.isEmpty());
    QVERIFY(gutterSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int parsedLineCount: 0")));
    QVERIFY(gutterSource.contains(QStringLiteral("property string sourceFilePath: \"\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("property string selectedNoteId: \"\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("property string selectedNoteDirectoryPath: \"\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("property real fallbackLineHeight: LV.Theme.gap20")));
    QVERIFY(gutterSource.contains(QStringLiteral("property var lineMetricProvider: null")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int lineMetricsRevision: 0")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int currentLineIndex: -1")));
    QVERIFY(gutterSource.contains(QStringLiteral("property color currentLineIndicatorColor: LV.Theme.accentBlue")));
    QVERIFY(gutterSource.contains(QStringLiteral("readonly property int minimumLineNumberDigitCount: 6")));
    QVERIFY(gutterSource.contains(QStringLiteral("readonly property bool hasCurrentLineIndicator")));
    QVERIFY(gutterSource.contains(QStringLiteral("gutter.minimumLineNumberDigitCount")));
    QVERIFY(gutterSource.contains(QStringLiteral("implicitWidth: LV.Theme.gap12 + gutter.lineNumberDigitCount * LV.Theme.gap8")));
    QVERIFY(gutterSource.contains(QStringLiteral("function lineMetricAt(lineIndex)")));
    QVERIFY(gutterSource.contains(QStringLiteral("readonly property bool hasSelectedSource")));
    QVERIFY(gutterSource.contains(QStringLiteral("gutter.contentY")));
    QVERIFY(gutterSource.contains(QStringLiteral("lineMetricProvider(normalizedIndex)")));
    QVERIFY(gutterSource.contains(QStringLiteral("lineMetric.y")));
    QVERIFY(gutterSource.contains(QStringLiteral("lineMetric.height")));
    QVERIFY(gutterSource.contains(QStringLiteral("objectName: \"contentsGutterCurrentLineIndicator\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("gutter.hasCurrentLineIndicator && index === gutter.currentLineIndex")));
    QVERIFY(gutterSource.contains(QStringLiteral("radius: width / 2")));
    QVERIFY(!gutterSource.contains(QStringLiteral("separatorColor")));
    QVERIFY(!gutterSource.contains(QStringLiteral("anchors.right: parent.right")));
    QVERIFY(textEditorSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(minimapSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property bool editorReadOnly: false")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property string noteBodyFilePath: \"\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property string editorDocumentText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("String(textEditor.text)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real viewportContentY")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorViewportHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorViewportContentHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorViewportWidth")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property real editorBottomViewportPaddingRatio: 0.75")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorBottomViewportPadding")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorLogicalLineHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property string editorPlainText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property int editorCursorLineIndex: textEditor.cursorLineIndexForLogicalCursor()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property int editorPlainTextRevision: 0")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property int editorLineMetricsRevision: 0")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorMeasuredContentHeight")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorRenderedLineCount")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorVisualLineHeight")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorLogicalLineCount")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function normalizedEditorPlainText(value)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function cursorLineIndexFor(documentText, cursorPosition)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorSurfacePlainText()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function cursorLineIndexForLogicalCursor()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function logicalLineStartPositionFor(lineIndex)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("slice(0, safeCursorPosition).split(\"\\n\").length - 1")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.getText(0, safeLength)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorLogicalLineMetricFor(lineIndex)")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("function editorLogicalLineMetricsFor(lineIndex)")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("function editorLineMetricsFor(lineIndex)")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("function buildEditorVisualLineMetrics(requiredCount, fallbackHeight)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function scrollEditorViewportTo(contentY)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function boundedCursorPosition(value)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function restoreEditorCursorPosition(nextCursorPosition)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.editorItem")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.positionAt(editorPoint.x, editorPoint.y)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.positionToRectangle")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.editorItem.contentHeight")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("- textEditor.editorBottomViewportPadding")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorSurface.lineCount")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("onWidthChanged: textEditor.bumpEditorPlainTextRevision()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function onContentHeightChanged()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function onLineCountChanged()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorViewportFlickable")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property: \"bottomPadding\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("value: textEditor.editorBottomViewportPadding")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function replaceEditorDocumentText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function pasteNativeClipboardText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.restoreEditorCursorPosition(nextCursorPosition);")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.paste()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(textEditorSource.contains(QStringLiteral("autoFocusOnPress: !LV.Theme.mobileTarget")));
    QVERIFY(textEditorSource.contains(QStringLiteral("enabled: LV.Theme.mobileTarget && !textEditor.readOnly")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function handleEditorPressEnded(eventData)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function handleEditorHoldStarted(eventData)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function focusEditorFromGesture(eventData)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("includeUiHit: true")));
    QVERIFY(textEditorSource.contains(QStringLiteral("trigger: \"pressEnded\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("trigger: \"holdStarted\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("eventData.finalInteractionKind")));
    QVERIFY(textEditorSource.contains(QStringLiteral("finalInteractionKind !== \"tap\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("maximumFingerCount > 1")));
    QVERIFY(textEditorSource.contains(QStringLiteral("eventData.scrollActive || eventData.dragActive")));
    QVERIFY(textEditorSource.contains(QStringLiteral("item.mapFromGlobal")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("TapHandler {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("preferNativeGestures: false")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("preferNativeGestures: LV.Theme.mobileTarget")));
    QVERIFY(minimapSource.contains(QStringLiteral("property string documentText: \"\"")));
    QVERIFY(minimapSource.contains(QStringLiteral("property real sourceContentY: 0")));
    QVERIFY(minimapSource.contains(QStringLiteral("property real sourceContentHeight")));
    QVERIFY(minimapSource.contains(QStringLiteral("property real sourceViewportHeight")));
    QVERIFY(minimapSource.contains(QStringLiteral("property real sourceContentWidth")));
    QVERIFY(minimapSource.contains(QStringLiteral("property var scrollTarget: null")));
    QVERIFY(minimapSource.contains(QStringLiteral("readonly property real minimapSizeFactor: 0.85")));
    QVERIFY(minimapSource.contains(QStringLiteral("implicitWidth: Math.max(LV.Theme.gap20 * 5, LV.Theme.gap12 * 8) * minimap.minimapSizeFactor")));
    QVERIFY(minimapSource.contains(QStringLiteral("* Math.max(0.01, LV.Theme.strokeThin / Math.max(1, LV.Theme.gap8))")));
    QVERIFY(minimapSource.contains(QStringLiteral("readonly property real minimapScale")));
    QVERIFY(minimapSource.contains(QStringLiteral("readonly property bool viewportOverlayVisible")));
    QVERIFY(minimapSource.contains(QStringLiteral("readonly property real viewportThumbY")));
    QVERIFY(minimapSource.contains(QStringLiteral("readonly property real viewportThumbHeight")));
    QVERIFY(minimapSource.contains(QStringLiteral("function requestScrollAtMinimapY(minimapY)")));
    QVERIFY(minimapSource.contains(QStringLiteral("TextEdit {")));
    QVERIFY(minimapSource.contains(QStringLiteral("textFormat: TextEdit.RichText")));
    QVERIFY(minimapSource.contains(QStringLiteral("readOnly: true")));
    QVERIFY(minimapSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(minimapSource.contains(QStringLiteral("minimapPointer.containsMouse || minimapPointer.pressed")));
    QVERIFY(minimapSource.contains(QStringLiteral("visible: minimap.viewportOverlayVisible")));
    QVERIFY(!minimapSource.contains(QStringLiteral("opacity: minimapPointer.containsMouse || minimapPointer.pressed ? 0.32 : 0.22")));
    QVERIFY(!minimapSource.contains(QStringLiteral("opacity: minimapPointer.containsMouse || minimapPointer.pressed ? 0.86 : 0.54")));
    QVERIFY(!minimapSource.contains(QStringLiteral("rowWidthRatios")));
    QVERIFY(!minimapSource.contains(QStringLiteral("rowCount")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("preferNativeGestures: true")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("property var documentModel")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("property var imeAdapter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("textDocumentModel")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorImeAdapter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("saveFile(")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("filePath: \"\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property var noteEditorSession: null")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property bool gutterVisible: true")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property string editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property int editorParsedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteEditorSession.editorFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteEditorSession.parsedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor.editorSelectionStart")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor.editorSelectionLength")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("replaceEditorDocumentText")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("importClipboardImageForEditor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("refreshClipboardImageAvailabilitySnapshot")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("pasteClipboardImageIntoEditor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("resourcesImportController")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property var inAppClipboard: null")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property var clipboardEditorPaste: null")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("pasteImageResourceIntoEditor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("importClipboardResourceForEditor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("insertImportedResourcesIntoSource")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("refreshClipboardResourceAvailabilitySnapshot")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("reloadImportedResources")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: StandardKey.Paste")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("context: Qt.ApplicationShortcut")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("handleEditorPasteShortcut")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("requestEditorPasteCommand")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("handleRuntimeEditorPasteKey")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorPasteShortcutMatches")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("target: LV.RuntimeEvents")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function onKeyPressed(key, modifiers, autoRepeat, text)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("pasteClipboardResourceIntoEditor")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function editorCommandShortcutEnabled()")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor.focused")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorItem.activeFocus")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("enabled: contentViewLayout.editorCommandShortcutEnabled()")));
    QVERIFY(!contentViewLayoutSource.contains(
        QStringLiteral("enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceFilePath: contentViewLayout.editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("selectedNoteId: contentViewLayout.editorActiveNoteId")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("selectedNoteDirectoryPath: contentViewLayout.editorActiveNoteDirectoryPath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineCount: contentViewLayout.editorParsedLineCount")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("lineCount: Math.max(contentViewLayout.editorParsedLineCount, contentsTextEditor.editorLogicalLineCount)")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("lineCount: contentsTextEditor.editorRenderedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentY: contentsTextEditor.viewportContentY")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("fallbackLineHeight: contentsTextEditor.editorLogicalLineHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineMetricProvider: contentsTextEditor.editorLogicalLineMetricFor")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineMetricsRevision: contentsTextEditor.editorLineMetricsRevision")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("currentLineIndex: contentsTextEditor.editorCursorLineIndex")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("Layout.preferredWidth: visible ? implicitWidth : 0")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("visible: contentViewLayout.gutterVisible")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("documentText: contentsTextEditor.editorDocumentText")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceContentY: contentsTextEditor.viewportContentY")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceContentHeight: contentsTextEditor.editorViewportContentHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceViewportHeight: contentsTextEditor.editorViewportHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceContentWidth: contentsTextEditor.editorViewportWidth")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("scrollTarget: contentsTextEditor.scrollEditorViewportTo")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteBodyFilePath: contentViewLayout.editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("persistEditorFile(path)")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("noteBodyFilePath: contentViewLayout.activeNoteBodyPath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Gutter {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.TextEditor {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Minimap {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("import WhatSon.App.Internal 1.0")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("CalendarView.")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("StackLayout")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorViewModeController")));

    const QList<QString> qmlSources = {gutterSource, textEditorSource, minimapSource};
    for (const QString &qmlSource : qmlSources) {
        QVERIFY(!qmlSource.contains(QStringLiteral("scaleMetric(")));
        QVERIFY(!qmlSource.contains(QStringLiteral("#")));
        const QRegularExpression hardcodedMetricAssignment(QStringLiteral(
            R"((?:^|\n)\s*(?:width|height|x|y|radius|anchors\.\w+Margin|Layout\.preferredWidth|implicitHeight|implicitWidth|font\.pixelSize)\s*:\s*-?\d)"));
        const QRegularExpressionMatch match = hardcodedMetricAssignment.match(qmlSource);
        QVERIFY2(!match.hasMatch(),
                 qPrintable(QStringLiteral("Hardcoded contents-view metric token: %1").arg(match.captured())));
    }
}

void WhatSonCppRegressionTests::qmlContentsView_threePartsStayViewOnlyAndNativeInputSafe()
{
    const QString gutterSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Gutter.qml"));
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));
    const QString minimapSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Minimap.qml"));
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));
    const QString combinedSource = gutterSource + textEditorSource + minimapSource;

    QVERIFY(!gutterSource.isEmpty());
    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!minimapSource.isEmpty());
    QVERIFY(!mainSource.isEmpty());
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("LV.CodeEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real viewportContentY")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorLogicalLineHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property int editorCursorLineIndex: textEditor.cursorLineIndexForLogicalCursor()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property real editorBottomViewportPaddingRatio: 0.75")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property: \"bottomPadding\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function scrollEditorViewportTo(contentY)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("autoFocusOnPress: !LV.Theme.mobileTarget")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorGestureMatchesEditor(eventData)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function focusEditorFromGesture(eventData)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("includeUiHit: true")));
    QVERIFY(textEditorSource.contains(QStringLiteral("trigger: \"pressEnded\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("trigger: \"holdStarted\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorPointFromGlobal(item, x, y)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorGestureUiMatchesEditor(eventData.originUi)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorLogicalLineMetricFor(lineIndex)")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("function editorLogicalLineMetricsFor(lineIndex)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function cursorLineIndexForLogicalCursor()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function logicalLineStartPositionFor(lineIndex)")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int parsedLineCount: 0")));
    QVERIFY(gutterSource.contains(QStringLiteral("property var lineMetricProvider: null")));
    QVERIFY(minimapSource.contains(QStringLiteral("TextEdit {")));
    QVERIFY(minimapSource.contains(QStringLiteral("function requestScrollAtMinimapY(minimapY)")));
    QVERIFY(!combinedSource.contains(QStringLiteral("import WhatSon.App.Internal 1.0")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsEditorDisplayBackend")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsStructuredDocumentFlow")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsResourceEditorView")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsMinimapLayoutMetrics")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsLineNumberRailMetrics")));
    QVERIFY(!combinedSource.contains(QStringLiteral("sourceText")));
    QVERIFY(!combinedSource.contains(QStringLiteral("renderedText")));
    QVERIFY(!combinedSource.contains(QStringLiteral("projection")));
    QVERIFY(!combinedSource.contains(QStringLiteral("persistence")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("TextEdit {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onReleased")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("textDocumentModel")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorImeAdapter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("saveFile(")));
    QVERIFY(mainSource.contains(QStringLiteral("desktopMainLayoutComponent")));
    QVERIFY(mainSource.contains(QStringLiteral("mobileMainLayoutComponent")));
    QVERIFY(mainSource.contains(QStringLiteral("BodyPanelView.BodyLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("BodyPanelView.StatusBarLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("BodyPanelView.NavigationBarLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("MobilePageView.MobileHierarchyPage {")));
    QVERIFY(!mainSource.contains(QStringLiteral("editorOnlyWorkspaceComponent")));
    QVERIFY(!mainSource.contains(QStringLiteral("BodyPanelView.ContentViewLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("rootEditorViewModeController")));
    QVERIFY(mainSource.contains(QStringLiteral("editorViewModeController: applicationWindow.rootEditorViewModeController")));
}

void WhatSonCppRegressionTests::qmlOnboardingContent_routesMacCreateHubThroughDirectoryDialog()
{
    const QString onboardingSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"));

    QVERIFY(!onboardingSource.isEmpty());
    QVERIFY(onboardingSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(onboardingSource.contains(
        QStringLiteral("readonly property bool useDirectoryCreateHubFlow: root.isMobilePlatform || Qt.platform.os === \"osx\"")));
    QVERIFY(!onboardingSource.contains(QStringLiteral("useMobileCreateDirectoryFlow")));
    QVERIFY(onboardingSource.contains(QStringLiteral("if (root.useDirectoryCreateHubFlow)\n                root.openCreateHubDirectoryDialog();")));
    QVERIFY(onboardingSource.contains(QStringLiteral("if (root.useDirectoryCreateHubFlow)\n            return null;")));
    QVERIFY(onboardingSource.contains(QStringLiteral("FolderDialog {\n        id: createHubDirectoryDialog")));
    QVERIFY(onboardingSource.contains(QStringLiteral("root.hubSessionController.createHubInDirectoryUrl(selectedFolder, root.requestedCreateHubFileName);")));
    QVERIFY(onboardingSource.contains(QStringLiteral("fileMode: FileDialog.SaveFile")));
    QVERIFY(onboardingSource.contains(QStringLiteral("root.createHubDialogInstance = createHubDialogComponent.createObject(root);")));
}

void WhatSonCppRegressionTests::qmlLvrsTokens_replaceDirectHardcodedVisualTokensOutsideContents()
{
    const QList<QString> tokenizedFiles = {
        QStringLiteral("src/app/qml/Main.qml"),
        QStringLiteral("src/app/qml/view/calendar/AgendaPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/CalendarEventCell.qml"),
        QStringLiteral("src/app/qml/view/calendar/CalendarTodayControl.qml"),
        QStringLiteral("src/app/qml/view/calendar/DayCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/MonthCalendarDayCell.qml"),
        QStringLiteral("src/app/qml/view/calendar/MonthCalendarGridSurface.qml"),
        QStringLiteral("src/app/qml/view/calendar/MonthCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/WeekCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/YearCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/panels/BodyLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/HierarchySidebarLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/ListBarHeader.qml"),
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/NoteListItem.qml"),
        QStringLiteral("src/app/qml/view/panels/ResourceListItem.qml"),
        QStringLiteral("src/app/qml/view/panels/StatusBarLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailContents.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailFileStatForm.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailMetadataHierarchyPicker.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/NoteDetailPanel.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/RightPanel.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/NavigationModeBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml"),
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"),
        QStringLiteral("src/app/qml/window/Onboarding.qml"),
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"),
        QStringLiteral("src/app/qml/window/TrialStatus.qml"),
    };
    const QRegularExpression hardcodedVisualToken(QStringLiteral(
        R"(#[0-9A-Fa-f]{3,8}|Qt\.rgba\(|color:\s*"transparent"|backgroundColor(?:Disabled|Focused|Hover|Pressed)?:\s*"transparent"|color:\s*"white"|(?:spacing|Layout\.minimumWidth|font\.pixelSize|insetVertical|topPadding|rightPadding|bottomPadding|leftPadding):\s*(?:0|10|22)\b)"));

    for (const QString &filePath : tokenizedFiles) {
        const QString source = readUtf8SourceFile(filePath);
        QVERIFY2(!source.isEmpty(), qPrintable(filePath));
        QVERIFY2(source.contains(QStringLiteral("import LVRS 1.0 as LV")), qPrintable(filePath));
        const QRegularExpressionMatch match = hardcodedVisualToken.match(source);
        QVERIFY2(!match.hasMatch(),
                 qPrintable(QStringLiteral("Hardcoded visual token in %1: %2").arg(filePath, match.captured())));
        if (filePath != QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml")
            && filePath != QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml")) {
            QVERIFY2(!source.contains(QStringLiteral("scaleMetric(")),
                     qPrintable(QStringLiteral("Unexpected scaled pixel literal in %1").arg(filePath)));
        }
    }
}
