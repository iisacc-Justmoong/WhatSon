#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/input/ContentsInlineFormatEditorController.hpp"
#include "app/models/editor/format/ContentsInlineStyleOverlayRenderer.hpp"
#include "app/models/editor/format/ContentsPlainTextSourceMutator.hpp"
#include "app/models/editor/geometry/ContentsEditorGeometryProvider.hpp"
#include "app/models/editor/lineNumber/ContentsLineNumberRailMetrics.hpp"
#include "app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp"
#include "app/models/editor/tags/ContentsEditorTagInsertionController.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRectF>

#include <memory>

namespace
{
    QString qmlInlineFormatEditorRepositoryRootPath()
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        return repositoryRoot.absolutePath();
    }

    QString qmlInlineFormatEditorErrorString(const QList<QQmlError>& errors)
    {
        QStringList messages;
        messages.reserve(errors.size());
        for (const QQmlError& error : errors)
        {
            messages.push_back(error.toString());
        }
        return messages.join(QLatin1Char('\n'));
    }

    void addWhatSonInlineFormatEditorQmlImportPaths(QQmlEngine& engine, const QString& repositoryRoot)
    {
        const QStringList candidatePaths{
            repositoryRoot + QStringLiteral("/src/app/qml"),
            repositoryRoot + QStringLiteral("/build/src/app"),
            repositoryRoot + QStringLiteral("/build/src/app/WhatSon"),
            repositoryRoot + QStringLiteral("/build/src/app/cmake/module"),
            repositoryRoot + QStringLiteral("/build/src/app/cmake/runtime/lvrs_runtime_qml"),
            repositoryRoot + QStringLiteral("/build/src/app/lvrs_runtime_qml"),
        };
        for (const QString& candidatePath : candidatePaths)
        {
            if (QFileInfo::exists(candidatePath))
            {
                engine.addImportPath(candidatePath);
            }
        }
    }

    void registerInlineFormatEditorRuntimeQmlTypes()
    {
        static const bool registered = []() {
            qmlRegisterType<ContentsInlineStyleOverlayRenderer>("WhatSon.App.Internal", 1, 0, "ContentsInlineStyleOverlayRenderer");
            qmlRegisterType<ContentsPlainTextSourceMutator>("WhatSon.App.Internal", 1, 0, "ContentsPlainTextSourceMutator");
            qmlRegisterType<ContentsEditorTagInsertionController>("WhatSon.App.Internal", 1, 0, "ContentsEditorTagInsertionController");
            qmlRegisterType<ContentsInlineFormatEditorController>("WhatSon.App.Internal", 1, 0, "ContentsInlineFormatEditorController");
            qmlRegisterType<ContentsEditorGeometryProvider>("WhatSon.App.Internal", 1, 0, "ContentsEditorGeometryProvider");
            qmlRegisterType<ContentsLineNumberRailMetrics>("WhatSon.App.Internal", 1, 0, "ContentsLineNumberRailMetrics");
            qmlRegisterType<ContentsEditorVisualLineMetrics>("WhatSon.App.Internal", 1, 0, "ContentsEditorVisualLineMetrics");
            return true;
        }();
        Q_UNUSED(registered);
    }

} // namespace

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsNativeTextEditInputUncovered()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("nativeTouchScrollGuardActive")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("TapHandler {")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("DragHandler {")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function setCursorPositionFromVisiblePoint(localX, localY)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function logicalPositionAtVisiblePoint(localX, localY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function logicalOffsetToSourceOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceOffsetForVisibleLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function restoreVisibleLogicalSelectionRange(anchorLogicalOffset, currentLogicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function restoreVisibleLogicalLineSelectionAtLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function restoreVisibleLogicalParagraphSelectionAtLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function syncRawSelectionFromSurfaceSelection()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function visibleLogicalOffsetAtPoint(localX, localY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Number(renderedGeometryProbe.contentHeight)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("positionAt(clampedX, clampedY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceTagTokenBoundsForCursor(sourceOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function normalizeCursorPositionAwayFromHiddenTagTokens()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: surfaceSelectionEditor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatSurfaceSelectionEditor\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral(
        "enabled: control.renderedOverlayVisible && !control.nativeCompositionActive()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByMouse: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readOnly: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onSelectionStartChanged: control.scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onSelectionEndChanged: control.scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("text: control.displayGeometryText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatVisibleSelectionPointerArea\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("hoverEnabled: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.updateVisiblePointerSelection(mouse.x, mouse.y)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function updateVisiblePointerClickSequence(localX, localY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (clickCount >= 3)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (clickCount === 2)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.restoreVisibleLogicalParagraphSelectionAtLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.restoreVisibleLogicalLineSelectionAtLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("acceptedButtons: Qt.LeftButton")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var logicalToSourceOffsets: []")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.onPressed: function (event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("key !== Qt.Key_Backspace")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function eventRequestsPasteShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function eventRequestsInlineFormatShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function eventRequestsBodyTagShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("const pureModifierKey = key === Qt.Key_Alt")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (pureModifierKey)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function triggerTagManagementShortcut(key, modifiers)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.accepted = false;")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sequence: \"Ctrl+Alt+C\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sequence: \"Meta+Alt+C\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sequence: \"Ctrl+Alt+A\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sequence: \"Meta+Alt+A\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+E\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sequence: \"Meta+Shift+E\"")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("sequence: \"Ctrl+H\"")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("sequence: \"Meta+H\"")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+H\"")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("sequence: \"Meta+Shift+H\"")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.Key_H")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.matches(StandardKey.Paste)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("!control.eventRequestsPasteShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("!control.eventRequestsInlineFormatShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("!control.eventRequestsBodyTagShortcut(event)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("handleMacModifierVerticalNavigation")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("activateInputAtPoint")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("insertTabIndentAsSpaces")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: renderedGeometryProbe")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("enabled: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("opacity: 0")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("activeFocusOnPress: control.autoFocusOnPress")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("anchors.leftMargin: LV.Theme.gap16")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("anchors.rightMargin: LV.Theme.gap16")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: renderedOverlay")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatRenderedOverlay\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("enabled: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readOnly: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textFormat: TextEdit.RichText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textMargin: LV.Theme.gapNone")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wrapMode: TextEdit.Wrap")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property color cursorColor: LV.Theme.accentBlue")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int logicalCursorPosition: textInput.cursorPosition")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int visiblePointerCursorLogicalOffset: -1")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property int resolvedProjectedCursorPosition")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.resolvedProjectedCursorPosition")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool visiblePointerCursorUpdateActive: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function cursorProjectionRectangle()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: projectedCursor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatProjectedCursor\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visible: control.projectedCursorVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("cursorDelegate: Rectangle {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeCursorVisible: !control.renderedOverlayVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatNativeCursor\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visible: control.nativeCursorVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("width: control.nativeCursorVisible ? control.cursorPixelWidth : 0")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("running: control.nativeCursorVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("insetHorizontal: LV.Theme.gapNone")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("insetVertical: LV.Theme.gapNone")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("insetVertical: LV.Theme.gap16")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int inputMethodHints: Qt.ImhNone")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int mouseSelectionMode: TextEdit.SelectCharacters")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeSelectionActive: textInput.selectionStart !== textInput.selectionEnd")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var normalizedHtmlBlocks: []")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool renderedOverlayVisible: control.renderedOverlayAvailable")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool renderedOverlayAvailable: control.showRenderedOutput")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool renderedResourceOverlayPinned: control.renderedOverlayAvailable")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function hasAtomicRenderedResourceBlocks()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("&& (!control.nativeCompositionActive() || control.renderedResourceOverlayPinned)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeSelectionContainsVisibleLogicalContent")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeSelectionPaintVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool renderedSelectionActive: control.renderedOverlayVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("&& control.nativeSelectionContainsVisibleLogicalContent")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceRangeContainsVisibleLogicalContent(range)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceOffsetIsInsideTagToken(text, sourceOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("&& !control.nativeSelectionActive")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectionStart === selectionEnd")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visible: control.renderedOverlayVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textColor: control.renderedOverlayVisible ? \"transparent\" : control.textColor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectedTextColor: control.renderedOverlayVisible ? \"transparent\" : control.textColor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectionColor: control.nativeSelectionPaintVisible ? LV.Theme.primaryOverlay : \"transparent\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("preferNativeGestures: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("renderedOverlay.select(selectionRange.start, selectionRange.end)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("htmlBlockObjectSource")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("contentsInlineFormatAtomicResourceSelectionLayer")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("moveCursorSelection(start, TextEdit.SelectCharacters)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("moveCursorSelection(end, TextEdit.SelectCharacters)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.restoreVisibleLogicalSelectionRange(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onCursorPositionChanged: control.normalizeCursorPositionAwayFromHiddenTagTokens()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool overwriteMode: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool persistentSelection: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByMouse: control.selectByMouse")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textInput: textInput.editorItem")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("showRenderedOutput: false")));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_projectsVisibleGeometryFromRenderedDisplay()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString documentFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property real displayContentHeight")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("? renderedOverlay.contentHeight")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property int visualLineCount")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var visualLineWidthRatios")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var logicalGutterRows: lineNumberRailMetrics.rows")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property int visualLineCount: visualLineMetrics.visualLineCount")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var visualLineWidthRatios: visualLineMetrics.visualLineWidthRatios")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("ContentsEditorVisualLineMetrics {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: visualLineMetrics")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("ContentsLineNumberRailMetrics {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: lineNumberRailMetrics")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("ContentsEditorGeometryProvider {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: editorGeometryProvider")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("measuredLineWidthRatios")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("measuredVisualLineCount")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("lineNumberRanges: lineNumberRailMetrics.logicalLineRanges")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("geometryRows")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textItem: control.displayGeometryItem()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visualItem: control.renderedOverlayVisible ? renderedOverlay : textInput.editorItem")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("resourceItem: renderedOverlay")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("targetItem: control")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sourceText: textInput.text")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("logicalText: control.displayGeometryText")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("geometryProvider:")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("textGeometryItem:")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("resourceGeometryItem:")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("visualLineMetrics.textItem")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function resolvedLogicalGutterRows()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function normalizedLogicalGutterBlocks()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function logicalLineSourceRangesForBlock(block)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function textDisplayRectangleForSourceRange(sourceStart, sourceEnd)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function resolvedVisibleLineCount()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function resolvedVisibleLineWidthRatios()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function resolvedTextItemLineWidthRatios(textItem)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textFormat: TextEdit.RichText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property string displayGeometryText: textInput.text")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function displayGeometryItem()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("return control.renderedOverlayVisible ? renderedGeometryProbe : textInput.editorItem")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function resourceDisplayRectangleForBlock(block)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("text: control.displayGeometryText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textFormat: TextEdit.PlainText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textMargin: LV.Theme.gapNone")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string logicalText: \"\"")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property var logicalToSourceOffsets: []")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property int logicalCursorPosition: sourceText.length")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property int editorVisualLineCount: editor.visualLineCount")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property var editorVisualLineWidthRatios: editor.visualLineWidthRatios")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property var editorLogicalGutterRows: editor.logicalGutterRows")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("displayGeometryText: documentFlow.logicalText.length > 0 ? documentFlow.logicalText : documentFlow.sourceText")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("logicalCursorPosition: documentFlow.logicalCursorPosition")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("logicalToSourceOffsets: documentFlow.logicalToSourceOffsets")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("normalizedHtmlBlocks: documentFlow.normalizedHtmlBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property real editorContentHeight: editor.displayContentHeight")));

    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsDisplayView.qml"));
    QVERIFY(displayViewSource.contains(QStringLiteral(
        "logicalCursorPosition: editorPresentationProjection.logicalOffsetForSourceOffset(")));
    QVERIFY(displayViewSource.contains(QStringLiteral(
        "visualLineCount: structuredDocumentFlow.editorVisualLineCount")));
    QVERIFY(displayViewSource.contains(QStringLiteral(
        "rowWidthRatios: structuredDocumentFlow.editorVisualLineWidthRatios")));
    QVERIFY(displayViewSource.contains(QStringLiteral(
        "rows: structuredDocumentFlow.editorLogicalGutterRows")));
    QVERIFY(!displayViewSource.contains(QStringLiteral(
        "logicalCursorPosition: editorPresentationProjection.logicalLengthForSourceText(")));
    QVERIFY(!displayViewSource.contains(QStringLiteral(
        "logicalLineCount: editorPresentationProjection.logicalLineCount")));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_positionsVisibleProbeFromLogicalDisplayText()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha\nBeta"
        logicalToSourceOffsets: [0, 7, 8, 9, 10, 18, 19, 20, 21, 22, 23]
        renderedText: "<span style='font-weight:700'>Alpha</span><br/>Beta"
        showRenderedOutput: true
        text: "<bold>Alpha</bold>\nBeta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatVisibleGeometryHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));

    QObject* projectedCursor = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatProjectedCursor"));
    QVERIFY(projectedCursor != nullptr);
    inlineEditor->setProperty("logicalCursorPosition", 5);
    QTRY_VERIFY(!inlineEditor->property("nativeCursorVisible").toBool());
    QTRY_VERIFY(projectedCursor->property("visible").toBool());
    QVERIFY(projectedCursor->property("width").toReal() >= 1.0);
    QVERIFY(projectedCursor->property("height").toReal() > 0.0);
    const qreal firstCursorY = projectedCursor->property("y").toReal();
    inlineEditor->setProperty("logicalCursorPosition", 6);
    QTRY_VERIFY(projectedCursor->property("y").toReal() > firstCursorY);

    inlineEditor->setProperty("showRenderedOutput", false);
    QTRY_VERIFY(inlineEditor->property("nativeCursorVisible").toBool());
    QTRY_VERIFY(!projectedCursor->property("visible").toBool());
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_mapsRenderedPointerSelectionToCharacterRawRange()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha beta"
        logicalToSourceOffsets: [0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 23]
        renderedText: "<span style='font-weight:700'>Alpha beta</span>"
        showRenderedOutput: true
        text: "<bold>Alpha beta</bold>"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatPointerSelectionHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreVisibleLogicalSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 2),
        Q_ARG(QVariant, 4)));
    QVERIFY(restoreResult.toBool());

    QTRY_COMPARE(inlineEditor->property("selectionStart").toInt(), 8);
    QCOMPARE(inlineEditor->property("selectionEnd").toInt(), 10);
    QCOMPARE(inlineEditor->property("selectedText").toString(), QStringLiteral("ph"));
    QVERIFY(inlineEditor->property("renderedSelectionActive").toBool());
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_movesRenderedCursorOnMouseClick()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha beta"
        logicalToSourceOffsets: [0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 23]
        renderedText: "<span style='font-weight:700'>Alpha beta</span>"
        showRenderedOutput: true
        text: "<bold>Alpha beta</bold>"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatPointerCursorHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QObject* surfaceSelectionEditor = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatSurfaceSelectionEditor"));
    QVERIFY(surfaceSelectionEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(surfaceSelectionEditor->property("enabled").toBool());

    inlineEditor->setProperty("cursorPosition", 0);
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), 0);
    const QRectF initialProjectedCursor =
        inlineEditor->property("projectedCursorRectangle").toRectF();

    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(76, 20));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());
    QTRY_VERIFY(inlineEditor->property("cursorPosition").toInt() > QStringLiteral("<bold>").size());
    QTRY_VERIFY(
        inlineEditor->property("projectedCursorRectangle").toRectF().x()
        > initialProjectedCursor.x() + 1.0);
    QVERIFY(!inlineEditor->property("nativeSelectionActive").toBool());

    const int clickedCursorPosition = inlineEditor->property("cursorPosition").toInt();
    QVERIFY(clickedCursorPosition > QStringLiteral("<bold>").size());
    QTest::keyClick(&window, Qt::Key_Right, Qt::ShiftModifier);
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), clickedCursorPosition);
    QVERIFY(inlineEditor->property("selectionEnd").toInt() > clickedCursorPosition);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_preservesRenderedPointerDragSelection()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha beta"
        logicalToSourceOffsets: [0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 23]
        renderedText: "<span style='font-weight:700'>Alpha beta</span>"
        showRenderedOutput: true
        text: "<bold>Alpha beta</bold>"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatPointerDragSelectionHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QObject* surfaceSelectionEditor = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatSurfaceSelectionEditor"));
    QVERIFY(surfaceSelectionEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(surfaceSelectionEditor->property("enabled").toBool());

    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(40, 20));
    QTest::mouseMove(&window, QPoint(140, 20), 20);
    QTest::qWait(40);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(140, 20));

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(inlineEditor->property("selectionEnd").toInt() > inlineEditor->property("selectionStart").toInt());
    QVERIFY(!inlineEditor->property("selectedText").toString().isEmpty());
    QTRY_VERIFY(inlineEditor->property("renderedSelectionActive").toBool());
    QVERIFY(!inlineEditor->property("projectedCursorVisible").toBool());
    QCOMPARE(inlineEditor->property("visiblePointerCursorLogicalOffset").toInt(), -1);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_restoresRenderedPointerMultiClickSelection()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 160

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha beta\nSecond line\n\nThird para"
        renderedText: "Alpha beta<br/>Second line<br/><br/>Third para"
        showRenderedOutput: true
        text: "Alpha beta\nSecond line\n\nThird para"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatPointerMultiClickSelectionHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 160);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());

    QVariant lineSelectionResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreVisibleLogicalLineSelectionAtLogicalOffset",
        Q_RETURN_ARG(QVariant, lineSelectionResult),
        Q_ARG(QVariant, 14)));
    QVERIFY(lineSelectionResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), 11);
    QCOMPARE(inlineEditor->property("selectionEnd").toInt(), 22);
    QCOMPARE(inlineEditor->property("selectedText").toString(), QStringLiteral("Second line"));
    QVERIFY(!inlineEditor->property("projectedCursorVisible").toBool());
    QCOMPARE(inlineEditor->property("visiblePointerCursorLogicalOffset").toInt(), -1);

    QVariant paragraphSelectionResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreVisibleLogicalParagraphSelectionAtLogicalOffset",
        Q_RETURN_ARG(QVariant, paragraphSelectionResult),
        Q_ARG(QVariant, 14)));
    QVERIFY(paragraphSelectionResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), 0);
    QCOMPARE(inlineEditor->property("selectionEnd").toInt(), 22);
    QVERIFY(inlineEditor->property("selectedText").toString().contains(QStringLiteral("Alpha beta")));
    QVERIFY(inlineEditor->property("selectedText").toString().contains(QStringLiteral("Second line")));
    QVERIFY(!inlineEditor->property("selectedText").toString().contains(QStringLiteral("Third para")));
    QVERIFY(!inlineEditor->property("projectedCursorVisible").toBool());
    QCOMPARE(inlineEditor->property("visiblePointerCursorLogicalOffset").toInt(), -1);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_skipsHiddenInlineTagsDuringNativeCursorMovement()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha beta"
        logicalCursorPosition: 0
        logicalToSourceOffsets: [0, 7, 8, 9, 10, 18, 19, 20, 21, 22, 23]
        renderedText: "<span style='font-weight:700'>Alpha</span> beta"
        showRenderedOutput: true
        text: "<bold>Alpha</bold> beta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatCursorSkipHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());

    inlineEditor->setProperty("previousRawCursorPosition", 0);
    inlineEditor->setProperty("cursorPosition", 1);
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), QStringLiteral("<bold>").size());

    const int closingTagStart = QStringLiteral("<bold>Alpha").size();
    const int closingTagEnd = closingTagStart + QStringLiteral("</bold>").size();
    inlineEditor->setProperty("previousRawCursorPosition", closingTagEnd);
    inlineEditor->setProperty("cursorPosition", closingTagEnd - 1);
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), closingTagStart);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_reportsWrappedVisualLineCountForMinimap()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 96
    height: 160

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha beta gamma delta epsilon zeta eta theta iota kappa"
        renderedText: "<span>Alpha beta gamma delta epsilon zeta eta theta iota kappa</span>"
        showRenderedOutput: true
        text: "Alpha beta gamma delta epsilon zeta eta theta iota kappa"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatVisualLineHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(96, 160);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QTRY_VERIFY(inlineEditor->property("visualLineCount").toInt() > 1);
    QTRY_VERIFY(!inlineEditor->property("logicalGutterRows").toList().isEmpty());
    const QVariantList logicalGutterRows = inlineEditor->property("logicalGutterRows").toList();
    QCOMPARE(logicalGutterRows.size(), 1);
    const QVariantMap firstGutterRow = logicalGutterRows.first().toMap();
    QCOMPARE(firstGutterRow.value(QStringLiteral("number")).toInt(), 1);
    QVERIFY(firstGutterRow.value(QStringLiteral("height")).toDouble() > 24.0);
    const QVariantList visualLineWidthRatios = inlineEditor->property("visualLineWidthRatios").toList();
    QVERIFY(visualLineWidthRatios.size() >= inlineEditor->property("visualLineCount").toInt());
    bool foundPartialWidthLine = false;
    for (const QVariant& ratio : visualLineWidthRatios)
    {
        const double normalizedRatio = ratio.toDouble();
        if (normalizedRatio > 0.0 && normalizedRatio < 0.95)
        {
            foundPartialWidthLine = true;
            break;
        }
    }
    QVERIFY(foundPartialWidthLine);

}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_placesLogicalGutterRowsAtIncreasingY()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 220
    height: 180

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha\nBeta\nGamma"
        renderedText: "<p>Alpha</p><p>Beta</p><p>Gamma</p>"
        showRenderedOutput: true
        text: "Alpha\nBeta\nGamma"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatLogicalGutterHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(220, 180);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QTRY_VERIFY(inlineEditor->property("logicalGutterRows").toList().size() == 3);

    const QVariantList logicalGutterRows = inlineEditor->property("logicalGutterRows").toList();
    const double firstY = logicalGutterRows.at(0).toMap().value(QStringLiteral("y")).toDouble();
    const double secondY = logicalGutterRows.at(1).toMap().value(QStringLiteral("y")).toDouble();
    const double thirdY = logicalGutterRows.at(2).toMap().value(QStringLiteral("y")).toDouble();
    QVERIFY2(
        secondY > firstY,
        qPrintable(QStringLiteral("Second gutter row must be below first row: %1 <= %2").arg(secondY).arg(firstY)));
    QVERIFY2(
        thirdY > secondY,
        qPrintable(QStringLiteral("Third gutter row must be below second row: %1 <= %2").arg(thirdY).arg(secondY)));
}

void WhatSonCppRegressionTests::qmlStructuredDocumentFlow_routesBottomBlankClickToBodyEnd()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 220

    EditorView.ContentsStructuredDocumentFlow {
        id: documentFlow
        objectName: "structuredDocumentFlowUnderTest"
        anchors.fill: parent
        editorSurfaceHtml: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        logicalText: "Alpha\nBeta"
        sourceText: "Alpha\nBeta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/StructuredDocumentFlowBottomClickHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 220);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* documentFlow = rootObject->findChild<QObject*>(QStringLiteral("structuredDocumentFlowUnderTest"));
    QVERIFY(documentFlow != nullptr);
    QTRY_VERIFY(documentFlow->property("editorContentHeight").toReal() > 0.0);
    QTRY_VERIFY(documentFlow->property("editorContentHeight").toReal() < 200.0);

    QVariant topHit;
    QVERIFY(QMetaObject::invokeMethod(
        documentFlow,
        "pointRequestsTerminalBodyClick",
        Q_RETURN_ARG(QVariant, topHit),
        Q_ARG(QVariant, 10),
        Q_ARG(QVariant, 1)));
    QVERIFY(!topHit.toBool());

    QVariant bottomHit;
    QVERIFY(QMetaObject::invokeMethod(
        documentFlow,
        "pointRequestsTerminalBodyClick",
        Q_RETURN_ARG(QVariant, bottomHit),
        Q_ARG(QVariant, 10),
        Q_ARG(QVariant, 210)));
    QVERIFY(bottomHit.toBool());

    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20, 210));
    QTRY_COMPARE(documentFlow->property("editorCursorPosition").toInt(), QStringLiteral("Alpha\nBeta").size());
}

void WhatSonCppRegressionTests::qmlStructuredDocumentFlow_routesBodyClickToRenderedCursorPosition()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 160

    EditorView.ContentsStructuredDocumentFlow {
        id: documentFlow
        objectName: "structuredDocumentFlowUnderTest"
        anchors.fill: parent
        editorSurfaceHtml: "<span style='font-weight:700'>Alpha beta</span>"
        logicalCursorPosition: 0
        logicalText: "Alpha beta"
        logicalToSourceOffsets: [0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 23]
        sourceText: "<bold>Alpha beta</bold>"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/StructuredDocumentFlowBodyClickHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 160);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* documentFlow = rootObject->findChild<QObject*>(QStringLiteral("structuredDocumentFlowUnderTest"));
    QVERIFY(documentFlow != nullptr);
    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("contentsStructuredDocumentInlineEditor"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(documentFlow->property("editorContentHeight").toReal() > 0.0);
    QTRY_COMPARE(documentFlow->property("editorCursorPosition").toInt(), 0);
    const QRectF initialProjectedCursor =
        inlineEditor->property("projectedCursorRectangle").toRectF();

    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(76, 20));
    QTRY_VERIFY(documentFlow->property("editorCursorPosition").toInt() > QStringLiteral("<bold>").size());
    QTRY_VERIFY(
        inlineEditor->property("projectedCursorRectangle").toRectF().x()
        > initialProjectedCursor.x() + 1.0);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_forwardsInlineFormatShortcutsToTagManagementHook()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    objectName: "inlineFormatShortcutHarness"
    property bool handled: false
    property int handledKey: -1
    property int handledModifiers: 0
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        text: "Alpha beta"
        tagManagementKeyPressHandler: function (event) {
            root.handled = true;
            root.handledKey = Number(event.key) || -1;
            root.handledModifiers = Number(event.modifiers) || 0;
            event.accepted = true;
            return true;
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatShortcutHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    QTest::keyClick(&window, Qt::Key_B, Qt::ControlModifier);

    QTRY_VERIFY(rootObject->property("handled").toBool());
    QCOMPARE(rootObject->property("handledKey").toInt(), static_cast<int>(Qt::Key_B));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::ControlModifier));

    rootObject->setProperty("handled", false);
    rootObject->setProperty("handledKey", -1);
    rootObject->setProperty("handledModifiers", 0);

    QTest::keyClick(&window, Qt::Key_E, Qt::ControlModifier | Qt::ShiftModifier);

    QTRY_VERIFY(rootObject->property("handled").toBool());
    QCOMPARE(rootObject->property("handledKey").toInt(), static_cast<int>(Qt::Key_E));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::ControlModifier));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::ShiftModifier));

    rootObject->setProperty("handled", false);
    rootObject->setProperty("handledKey", -1);
    rootObject->setProperty("handledModifiers", 0);

    QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier | Qt::AltModifier);

    QTRY_VERIFY(rootObject->property("handled").toBool());
    QCOMPARE(rootObject->property("handledKey").toInt(), static_cast<int>(Qt::Key_C));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::ControlModifier));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::AltModifier));

    rootObject->setProperty("handled", false);
    rootObject->setProperty("handledKey", -1);
    rootObject->setProperty("handledModifiers", 0);

    QTest::keyClick(&window, Qt::Key_A, Qt::ControlModifier | Qt::AltModifier);

    QTRY_VERIFY(rootObject->property("handled").toBool());
    QCOMPARE(rootObject->property("handledKey").toInt(), static_cast<int>(Qt::Key_A));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::ControlModifier));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::AltModifier));
}

void WhatSonCppRegressionTests::qmlStructuredDocumentFlow_appliesInlineFormatShortcutToSelectedRawRange()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    objectName: "structuredDocumentFlowFormattingHarness"
    property string committedText: ""
    width: 360
    height: 96

    EditorView.ContentsStructuredDocumentFlow {
        id: documentFlow
        objectName: "structuredDocumentFlowUnderTest"
        anchors.fill: parent
        editorSurfaceHtml: sourceText
        logicalText: sourceText
        sourceText: "Alpha beta"

        onSourceTextEdited: function (text) {
            root.committedText = text;
            documentFlow.sourceText = text;
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/StructuredDocumentFlowFormattingHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* documentFlow = rootObject->findChild<QObject*>(QStringLiteral("structuredDocumentFlowUnderTest"));
    QVERIFY(documentFlow != nullptr);
    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("contentsStructuredDocumentInlineEditor"));
    QVERIFY(inlineEditor != nullptr);
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, 5),
        Q_ARG(QVariant, 5)));
    QVERIFY(restoreResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());

    QTest::keyClick(&window, Qt::Key_B, Qt::ControlModifier);

    QTRY_COMPARE(rootObject->property("committedText").toString(), QStringLiteral("<bold>Alpha</bold> beta"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("<bold>Alpha</bold> beta"));
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), QStringLiteral("<bold>").size());
    QCOMPARE(
        inlineEditor->property("selectionEnd").toInt(),
        QStringLiteral("<bold>Alpha").size());

    rootObject->setProperty("committedText", QString());
    QTest::keyClick(&window, Qt::Key_U, Qt::ControlModifier);

    QTRY_COMPARE(
        rootObject->property("committedText").toString(),
        QStringLiteral("<bold><underline>Alpha</underline></bold> beta"));
    QTRY_COMPARE(
        inlineEditor->property("text").toString(),
        QStringLiteral("<bold><underline>Alpha</underline></bold> beta"));
    QVERIFY(!rootObject->property("committedText").toString().contains(QStringLiteral("<underline></underline>")));

    rootObject->setProperty("committedText", QString());
    documentFlow->setProperty("sourceText", QStringLiteral("Look here"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("Look here"));
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, 4),
        Q_ARG(QVariant, 4)));
    QVERIFY(restoreResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());

    QTest::keyClick(&window, Qt::Key_U, Qt::ControlModifier);
    QTRY_COMPARE(rootObject->property("committedText").toString(), QStringLiteral("<underline>Look</underline> here"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("<underline>Look</underline> here"));

    rootObject->setProperty("committedText", QString());
    QTest::keyClick(&window, Qt::Key_B, Qt::ControlModifier);
    QTRY_COMPARE(
        rootObject->property("committedText").toString(),
        QStringLiteral("<underline><bold>Look</bold></underline> here"));
    QTRY_COMPARE(
        inlineEditor->property("text").toString(),
        QStringLiteral("<underline><bold>Look</bold></underline> here"));
    QVERIFY(!rootObject->property("committedText").toString().contains(QStringLiteral("<bold></bold>")));

    rootObject->setProperty("committedText", QString());
    documentFlow->setProperty("sourceText", QStringLiteral("Alpha\nBeta"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("Alpha\nBeta"));
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, 5),
        Q_ARG(QVariant, 5)));
    QVERIFY(restoreResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());

    QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier | Qt::AltModifier);

    QTRY_COMPARE(rootObject->property("committedText").toString(), QStringLiteral("<callout>Alpha</callout>\nBeta"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("<callout>Alpha</callout>\nBeta"));

    rootObject->setProperty("committedText", QString());
    documentFlow->setProperty("sourceText", QStringLiteral("Meta\nCallout"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("Meta\nCallout"));
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, 4),
        Q_ARG(QVariant, 4)));
    QVERIFY(restoreResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());

    QTest::keyClick(&window, Qt::Key_C, Qt::MetaModifier | Qt::AltModifier);

    QTRY_COMPARE(rootObject->property("committedText").toString(), QStringLiteral("<callout>Meta</callout>\nCallout"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("<callout>Meta</callout>\nCallout"));

    rootObject->setProperty("committedText", QString());
    documentFlow->setProperty("sourceText", QStringLiteral("Intro"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("Intro"));
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 5),
        Q_ARG(QVariant, 5),
        Q_ARG(QVariant, 5)));
    QVERIFY(restoreResult.toBool());

    QTest::keyClick(&window, Qt::Key_A, Qt::ControlModifier | Qt::AltModifier);

    QTRY_VERIFY(rootObject->property("committedText").toString().startsWith(QStringLiteral("Intro\n<agenda date=\"")));
    QVERIFY(rootObject->property("committedText").toString().contains(QStringLiteral("<task done=\"false\"> </task></agenda>")));

    rootObject->setProperty("committedText", QString());
    documentFlow->setProperty("sourceText", QStringLiteral("Meta agenda"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("Meta agenda"));
    const int metaAgendaEnd = QStringLiteral("Meta agenda").size();
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, metaAgendaEnd),
        Q_ARG(QVariant, metaAgendaEnd),
        Q_ARG(QVariant, metaAgendaEnd)));
    QVERIFY(restoreResult.toBool());

    QTest::keyClick(&window, Qt::Key_A, Qt::MetaModifier | Qt::AltModifier);

    QTRY_VERIFY(rootObject->property("committedText").toString().startsWith(QStringLiteral("Meta agenda\n<agenda date=\"")));
    QVERIFY(rootObject->property("committedText").toString().contains(QStringLiteral("<task done=\"false\"> </task></agenda>")));

    rootObject->setProperty("committedText", QString());
    documentFlow->setProperty("sourceText", QStringLiteral("Glow text"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("Glow text"));
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, 4),
        Q_ARG(QVariant, 4)));
    QVERIFY(restoreResult.toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());

    QTest::keyClick(&window, Qt::Key_E, Qt::ControlModifier | Qt::ShiftModifier);

    QTRY_COMPARE(rootObject->property("committedText").toString(), QStringLiteral("<highlight>Glow</highlight> text"));
    QTRY_COMPARE(inlineEditor->property("text").toString(), QStringLiteral("<highlight>Glow</highlight> text"));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsRenderedOverlayDuringNativeSelection()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    readonly property string resourceTag: "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/demo.wsresource\" id=\"demo\" />"
    readonly property int resourceStart: 6
    readonly property int resourceEnd: resourceStart + resourceTag.length
    width: 360
    height: 220

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha\n\nBeta"
        logicalToSourceOffsets: [
            0, 1, 2, 3, 4, 5, 6,
            root.resourceEnd + 1,
            root.resourceEnd + 2,
            root.resourceEnd + 3,
            root.resourceEnd + 4,
            root.resourceEnd + 5
        ]
        normalizedHtmlBlocks: [{
            "htmlBlockIsDisplayBlock": true,
            "htmlBlockObjectSource": "iiHtmlBlock",
            "renderDelegateType": "resource",
            "sourceStart": root.resourceStart,
            "sourceEnd": root.resourceEnd
        }]
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'><img src='' width='120' height='60' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        showRenderedOutput: true
        text: "Alpha\n" + root.resourceTag + "\nBeta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatSelectionHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QObject* renderedOverlay = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatRenderedOverlay"));
    QVERIFY(renderedOverlay != nullptr);
    QVERIFY(!renderedOverlay->property("text").toString().contains(QStringLiteral("<resource")));
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, 4096),
        Q_ARG(QVariant, 4096)));
    QVERIFY(restoreResult.toBool());

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(inlineEditor->property("selectedText").toString().contains(QStringLiteral("<resource type=\"image\"")));
    QTRY_VERIFY(
        renderedOverlay->property("selectionEnd").toInt()
        > renderedOverlay->property("selectionStart").toInt());
    QTRY_COMPARE(inlineEditor->property("atomicResourceSelectionRects").toList().size(), 1);
    const QVariantMap resourceSelectionRect =
        inlineEditor->property("atomicResourceSelectionRects").toList().constFirst().toMap();
    QVERIFY(resourceSelectionRect.value(QStringLiteral("height")).toReal() > 20.0);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_ignoresEmptyFormattingTagsDuringRenderedSelection()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    readonly property string emptyFormattingTags: "<highlight><highlight></highlight></highlight>"
    readonly property int emptyFormattingTagsLength: emptyFormattingTags.length
    width: 360
    height: 120

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Next"
        logicalToSourceOffsets: [
            0,
            root.emptyFormattingTagsLength + 1,
            root.emptyFormattingTagsLength + 2,
            root.emptyFormattingTagsLength + 3,
            root.emptyFormattingTagsLength + 4
        ]
        renderedText: "Next"
        showRenderedOutput: true
        text: root.emptyFormattingTags + "Next"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatEmptyTagSelectionHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QObject* renderedOverlay = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatRenderedOverlay"));
    QVERIFY(renderedOverlay != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    const int emptyFormattingTagsLength = rootObject->property("emptyFormattingTagsLength").toInt();
    QVERIFY(emptyFormattingTagsLength > 0);

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 0),
        Q_ARG(QVariant, emptyFormattingTagsLength),
        Q_ARG(QVariant, emptyFormattingTagsLength)));
    QVERIFY(restoreResult.toBool());

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(inlineEditor->property("selectedText").toString().contains(QStringLiteral("<highlight>")));
    QTRY_VERIFY(!inlineEditor->property("nativeSelectionContainsVisibleLogicalContent").toBool());
    QTRY_VERIFY(!inlineEditor->property("nativeSelectionPaintVisible").toBool());
    QTRY_VERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
    QTRY_VERIFY(
        renderedOverlay->property("selectionEnd").toInt()
        == renderedOverlay->property("selectionStart").toInt());
    QTRY_COMPARE(inlineEditor->property("atomicResourceSelectionRects").toList().size(), 0);

    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 1),
        Q_ARG(QVariant, emptyFormattingTagsLength - 1),
        Q_ARG(QVariant, emptyFormattingTagsLength - 1)));
    QVERIFY(restoreResult.toBool());

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QTRY_VERIFY(!inlineEditor->property("nativeSelectionContainsVisibleLogicalContent").toBool());
    QTRY_VERIFY(!inlineEditor->property("nativeSelectionPaintVisible").toBool());
    QTRY_VERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
    QTRY_VERIFY(
        renderedOverlay->property("selectionEnd").toInt()
        == renderedOverlay->property("selectionStart").toInt());

    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, emptyFormattingTagsLength),
        Q_ARG(QVariant, emptyFormattingTagsLength + 4),
        Q_ARG(QVariant, emptyFormattingTagsLength + 4)));
    QVERIFY(restoreResult.toBool());

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionContainsVisibleLogicalContent").toBool());
    QTRY_VERIFY(inlineEditor->property("nativeSelectionPaintVisible").toBool());
    QTRY_VERIFY(inlineEditor->property("renderedSelectionActive").toBool());
    QTRY_VERIFY(
        renderedOverlay->property("selectionEnd").toInt()
        > renderedOverlay->property("selectionStart").toInt());
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsRenderedOverlayPassiveForNativeEditing()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        renderedText: "<b>Alpha</b> beta"
        showRenderedOutput: true
        text: "Alpha beta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatPassiveOverlayHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QObject* renderedOverlay = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatRenderedOverlay"));
    QVERIFY(renderedOverlay != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(!renderedOverlay->property("enabled").toBool());

    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(32, 24));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    inlineEditor->setProperty("cursorPosition", 0);
    QTest::keyClick(&window, Qt::Key_Right, Qt::ShiftModifier);
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), 0);
    QVERIFY(inlineEditor->property("selectionEnd").toInt() > inlineEditor->property("selectionStart").toInt());
    QVERIFY(!inlineEditor->property("selectedText").toString().isEmpty());

    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "clearSelection"));
    QTest::keyClick(&window, Qt::Key_Z);
    QTRY_VERIFY(inlineEditor->property("text").toString().contains(QLatin1Char('z'), Qt::CaseInsensitive));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsResourceOverlayPinnedDuringNativeEditing()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    readonly property string resourceTag: "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/demo.wsresource\" id=\"demo\" />"
    readonly property int resourceStart: 6
    readonly property int resourceEnd: resourceStart + resourceTag.length
    width: 360
    height: 160

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        displayGeometryText: "Alpha\n\nBeta"
        logicalToSourceOffsets: [
            0, 1, 2, 3, 4, 5, 6,
            root.resourceEnd + 1,
            root.resourceEnd + 2,
            root.resourceEnd + 3,
            root.resourceEnd + 4,
            root.resourceEnd + 5
        ]
        normalizedHtmlBlocks: [{
            "htmlBlockIsDisplayBlock": true,
            "htmlBlockObjectSource": "iiHtmlBlock",
            "renderDelegateType": "resource",
            "sourceStart": root.resourceStart,
            "sourceEnd": root.resourceEnd
        }]
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'><img src='' width='120' height='60' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        showRenderedOutput: true
        text: "Alpha\n" + root.resourceTag + "\nBeta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatPinnedResourceOverlayHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 120);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QObject* renderedOverlay = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatRenderedOverlay"));
    QVERIFY(renderedOverlay != nullptr);

    QVERIFY(!renderedOverlay->property("text").toString().contains(QStringLiteral("<resource")));
    QTRY_VERIFY(inlineEditor->property("renderedResourceOverlayPinned").toBool());
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());

    QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus");
    QTRY_VERIFY(inlineEditor->property("focused").toBool());
    QTest::keyClick(&window, Qt::Key_Z);

    QTRY_VERIFY(inlineEditor->property("text").toString().contains(QLatin1Char('z'), Qt::CaseInsensitive));
    QVERIFY(inlineEditor->property("text").toString().contains(QStringLiteral("<resource type=\"image\"")));
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(!renderedOverlay->property("text").toString().contains(QStringLiteral("<resource")));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsKeyboardSelectionAndOsImeNative()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString inlineEditorControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.hpp"));
    const QString inlineEditorControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.cpp"));
    const QString inlineEditorControllerHelperSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditorController.qml"));
    const QString surfaceGuardHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsEditorSurfaceGuardController.hpp"));
    const QString resourceImportControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.hpp"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerHeader.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY(!inlineEditorControllerHelperSource.isEmpty());
    QVERIFY(!surfaceGuardHeader.isEmpty());
    QVERIFY(!resourceImportControllerHeader.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool selectByKeyboard: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("inputMethodHints: control.inputMethodHints")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("mouseSelectionMode: control.mouseSelectionMode")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("overwriteMode: control.overwriteMode")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("persistentSelection: control.persistentSelection")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByKeyboard: control.selectByKeyboard")));
    QVERIFY(inlineEditorControllerHeader.contains(QStringLiteral("class ContentsInlineFormatEditorController")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("ContentsInlineFormatEditorController::nativeCompositionActive()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool inputMethodComposing: textInput.inputMethodComposing")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property string preeditText: String(textInput.editorItem.preeditText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function programmaticTextSyncPolicy(nextText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("return inlineEditorController.programmaticTextSyncPolicy(nextText);")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool preferNativeInputHandling: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("inlineEditorController.setProgrammaticText(resolvedText)")));
    QVERIFY(inlineEditorControllerHelperSource.contains(QStringLiteral("ContentsEditorInputPolicyAdapter")));
    QVERIFY(inlineEditorControllerHelperSource.contains(QStringLiteral("property bool localSelectionInteractionSinceFocus")));
    QVERIFY(inlineEditorControllerHelperSource.contains(QStringLiteral("function nativeSelectionActive()")));
    QVERIFY(inlineEditorControllerHelperSource.contains(QStringLiteral("helper.localSelectionInteractionSinceFocus || nativeSelectionActive()")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("ContentsInlineFormatEditorController::programmaticTextSyncPolicy")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("invokeHelperVariantMap(\"programmaticTextSyncPolicy\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.onPressed: function (event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("key !== Qt.Key_Backspace")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.eventRequestsPasteShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.eventRequestsInlineFormatShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.eventRequestsBodyTagShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.matches(StandardKey.Paste)")));
    QVERIFY(surfaceGuardHeader.contains(QStringLiteral("class ContentsEditorSurfaceGuardController")));
    QVERIFY(resourceImportControllerHeader.contains(QStringLiteral("editorInputPolicyAdapterChanged")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("notifyInputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("notifyQInputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("inputMethodVisible")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod &&")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod.visible !== undefined")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.ImQuery")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("setCursorPositionPreservingInputMethod")));

    const QDir repositoryRoot(qmlInlineFormatEditorRepositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QStringList forbiddenInputMethodPatterns{
        QStringLiteral("Qt.inputMethod"),
        QStringLiteral("InputMethod."),
        QStringLiteral("notifyInputMethod"),
        QStringLiteral("notifyQInputMethod"),
        QStringLiteral("inputMethodVisible"),
        QStringLiteral("Qt.inputMethod &&"),
        QStringLiteral("Qt.inputMethod.visible !== undefined"),
        QStringLiteral("Qt.ImQuery"),
    };
    const QStringList qmlRoots{
        QStringLiteral("src/app/qml"),
        QStringLiteral("src/app/models/editor"),
        QStringLiteral("src/app/models/editor"),
    };
    for (const QString& qmlRoot : qmlRoots)
    {
        QDirIterator qmlFiles(
            repositoryRoot.filePath(qmlRoot),
            QStringList{QStringLiteral("*.qml")},
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (qmlFiles.hasNext())
        {
            const QString absolutePath = qmlFiles.next();
            QFile sourceFile(absolutePath);
            QVERIFY2(sourceFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(absolutePath));
            const QString qmlSource = QString::fromUtf8(sourceFile.readAll());
            for (const QString& forbiddenPattern : forbiddenInputMethodPatterns)
            {
                QVERIFY2(
                    !qmlSource.contains(forbiddenPattern),
                    qPrintable(QStringLiteral("%1 contains forbidden input-method pattern: %2")
                                   .arg(repositoryRoot.relativeFilePath(absolutePath), forbiddenPattern)));
            }
        }
    }
}
