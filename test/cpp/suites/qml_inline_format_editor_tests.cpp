#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/input/ContentsInlineFormatEditorController.hpp"
#include "app/models/editor/input/ContentsWysiwygEditorPolicy.hpp"
#include "app/models/editor/format/ContentsInlineStyleOverlayRenderer.hpp"
#include "app/models/editor/format/ContentsPlainTextSourceMutator.hpp"
#include "app/models/editor/geometry/ContentsEditorGeometryProvider.hpp"
#include "app/models/editor/lineNumber/ContentsLineNumberRailMetrics.hpp"
#include "app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp"
#include "app/models/editor/projection/ContentsEditorPresentationProjection.hpp"
#include "app/models/editor/tags/ContentsEditorTagInsertionController.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QColor>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QImage>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRectF>
#include <QTemporaryDir>

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
            qmlRegisterType<ContentsWysiwygEditorPolicy>("WhatSon.App.Internal", 1, 0, "ContentsWysiwygEditorPolicy");
            qmlRegisterType<ContentsEditorGeometryProvider>("WhatSon.App.Internal", 1, 0, "ContentsEditorGeometryProvider");
            qmlRegisterType<ContentsLineNumberRailMetrics>("WhatSon.App.Internal", 1, 0, "ContentsLineNumberRailMetrics");
            qmlRegisterType<ContentsEditorVisualLineMetrics>("WhatSon.App.Internal", 1, 0, "ContentsEditorVisualLineMetrics");
            qmlRegisterType<ContentsEditorPresentationProjection>("WhatSon.App.Internal", 1, 0, "ContentsEditorPresentationProjection");
            return true;
        }();
        Q_UNUSED(registered);
    }

} // namespace

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsNativeTextEditInputUncovered()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString wysiwygPolicySource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsWysiwygEditorPolicy.cpp"));

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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.forceActiveFocus();")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function visibleLogicalOffsetAtPoint(localX, localY)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function applyRenderedBackspaceMutation(event)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.visibleBackspaceMutationPayload(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.visibleTextMutationPayload(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool logicalSurfaceActive")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property int sourceCursorPosition")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("\"logicalCursorPosition\": textInput.cursorPosition")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("\"sourceCursorPosition\": control.sourceCursorPosition")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("\"cursorPosition\": control.sourceCursorPosition")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("resolvedProjectedCursorPosition")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("visiblePointerCursorLogicalOffset")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("visiblePointerCursorUpdateActive")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("contentsInlineFormatProjectedCursor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.handleTagManagementKeyPress(event);")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("const geometryItem = control.displayGeometryItem()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Number(geometryItem.contentHeight)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("positionAt(clampedX, clampedY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceTagTokenBoundsForCursor(sourceOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function normalizeCursorPositionAwayFromHiddenTagTokens()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: surfaceSelectionEditor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatSurfaceSelectionEditor\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("enabled: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByMouse: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readOnly: true")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("onSelectionStartChanged: control.scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("onSelectionEndChanged: control.scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("onCursorPositionChanged: control.scheduleSurfaceSelectionToRawSync()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int surfaceSelectionStartLogicalOffset: 0")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int surfaceSelectionEndLogicalOffset: 0")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int surfaceSelectionCursorLogicalOffset: 0")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("text: control.displayGeometryText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textFormat: TextEdit.PlainText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function atomicResourceHitAtPoint(localX, localY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function visibleLogicalSelectionEndpointAtPoint(localX, localY, anchorLogicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("lineNumberRailMetrics.logicalLineRanges")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("editorGeometryProvider.lineNumberGeometryRows")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatVisibleSelectionPointerArea\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("mouse.accepted = control.beginVisiblePointerSelection(mouse.x, mouse.y);")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.updateVisiblePointerSelection(mouse.x, mouse.y);")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function updateVisiblePointerClickSequence(localX, localY)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (clickCount >= 3)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (clickCount === 2)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.restoreVisibleLogicalParagraphSelectionAtLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.restoreVisibleLogicalLineSelectionAtLogicalOffset(logicalOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var coordinateMapper: null")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var logicalToSourceOffsets")));
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
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("text: control.projectedNativeSurfaceText()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Component.onCompleted: control.syncNativeSurfaceTextFromProjection(true)")));
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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property alias logicalCursorPosition: textInput.cursorPosition")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function cursorProjectionRectangle()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("id: projectedCursor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("cursorDelegate: Rectangle {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool atomicResourceCursorActive")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("&& !control.atomicResourceCursorActive")));
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
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("&& (!control.nativeCompositionActive() || control.renderedResourceOverlayPinned)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeSelectionContainsVisibleLogicalContent")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool nativeSelectionPaintVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool renderedSelectionActive: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceRangeContainsVisibleLogicalContent(range)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function sourceOffsetIsInsideTagToken(text, sourceOffset)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("&& !control.nativeSelectionActive")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visible: control.renderedOverlayVisible")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textColor: control.renderedOverlayVisible ? \"transparent\" : control.textColor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectedTextColor: control.renderedOverlayVisible ? \"transparent\" : control.textColor")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectionColor: control.nativeSelectionPaintVisible ? LV.Theme.primaryOverlay : \"transparent\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("preferNativeGestures: true")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("renderedOverlay.select(selectionRange.start, selectionRange.end)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("contentsInlineFormatAtomicResourceSelectionLayer")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("ContentsWysiwygEditorPolicy {")));
    QVERIFY(wysiwygPolicySource.contains(QStringLiteral("htmlBlockObjectSource")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("moveCursorSelection(start, TextEdit.SelectCharacters)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("moveCursorSelection(end, TextEdit.SelectCharacters)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.restoreVisibleLogicalSelectionRange(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onCursorPositionChanged: {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.normalizeCursorPositionAwayFromHiddenTagTokens();")));
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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property real displayTextContentHeight")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("? Math.max(renderedOverlay.contentHeight, control.structuredVisualContentHeight)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("+ control.editorBottomInset")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property real displayBodyHeight: Math.max(0, control.displayTextContentHeight)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property int visualLineCount")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var visualLineWidthRatios")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var logicalGutterRows: lineNumberRailMetrics.rows")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var lineNumberGeometryRows: editorGeometryProvider.lineNumberGeometryRows")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var lineNumberGeometryResourceBlockHeights: control.resourceVisualBlockHeights()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property int visualLineCount: visualLineMetrics.visualLineCount")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property var visualLineWidthRatios: visualLineMetrics.visualLineWidthRatios")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var documentBlocks: []")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var resourceVisualBlocks: []")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function resourceVisualLayoutRows()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatResourceVisualBlock\"")));
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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("resourceVisualBlocks: control.resourceVisualBlocks")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("renderedHtml: control.renderedText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textItem: control.displayGeometryItem()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("visualItem: control.renderedOverlayVisible ? renderedOverlay : textInput.editorItem")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("resourceItem: renderedOverlay")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("targetItem: control")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("sourceText: control.text")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("logicalText: control.displayGeometryText")));
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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property string displayGeometryText: control.text")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property real editorBottomInset: LV.Theme.gap16")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function displayGeometryItem()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("return control.logicalSurfaceActive ? renderedGeometryProbe : textInput.editorItem;")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function lineNumberGeometryItem()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function resourceDisplayRectangleForBlock(block)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function resourceSelectionRectangleForBlock(block)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("function buildAtomicResourceSelectionRects()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("objectName: \"contentsInlineFormatRenderedGeometryProbe\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("text: control.displayGeometryText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textFormat: TextEdit.PlainText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textMargin: LV.Theme.gapNone")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string logicalText: \"\"")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string projectionSourceText: documentFlow.sourceText")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property bool logicalProjectionReady")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property string lastReadyEditorSurfaceHtml: \"\"")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property var documentBlocks: []")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property var resourceVisualBlocks: []")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property string resolvedEditorSurfaceHtml")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property string resolvedDisplayGeometryText")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function refreshLastReadyProjection()")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("renderedText: documentFlow.resolvedEditorSurfaceHtml")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("property var coordinateMapper: null")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("property var logicalToSourceOffsets")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("property int logicalCursorPosition: sourceText.length")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("logicalCursorPosition: documentFlow.logicalCursorPosition")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property int editorVisualLineCount: editor.visualLineCount")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property var editorVisualLineWidthRatios: editor.visualLineWidthRatios")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property var editorLogicalGutterRows: editor.logicalGutterRows")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("displayGeometryText: documentFlow.resolvedDisplayGeometryText")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("displayGeometryText: documentFlow.logicalText.length > 0 ? documentFlow.logicalText : documentFlow.sourceText")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("coordinateMapper: documentFlow.coordinateMapper")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("function terminalBodySurfaceY()")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("Number(editor.displayBodyHeight)")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("y: documentFlow.terminalBodySurfaceY()")));
    QVERIFY(!documentFlowSource.contains(QStringLiteral("logicalToSourceOffsets: documentFlow.logicalToSourceOffsets")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("normalizedHtmlBlocks: documentFlow.normalizedHtmlBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("documentBlocks: documentFlow.documentBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("resourceVisualBlocks: documentFlow.resourceVisualBlocks")));
    QVERIFY(documentFlowSource.contains(QStringLiteral("readonly property real editorContentHeight: editor.displayContentHeight")));

    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "editorCursorPosition: structuredDocumentFlow.editorCursorPosition")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral(
        "logicalCursorPosition: editorDisplayBackend.presentationProjection.logicalCursorPosition")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "coordinateMapper: editorDisplayBackend.presentationProjection")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "sourceText: editorDisplayBackend.editorSession.editorText")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "projectionSourceText: editorDisplayBackend.presentationProjection.sourceText")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "documentBlocks: editorDisplayBackend.structuredBlockRenderer.renderedDocumentBlocks")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "resourceVisualBlocks: editorDisplayBackend.inlineResourceVisualBlocks(")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral(
        "resourceVisualHeights: editorDisplayBackend.inlineResourceVisualHeights(")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("renderInlineResourceEditorSurfaceHtml(")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("logicalToSourceOffsets")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "property: \"visualLineCount\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "rowWidthRatios: structuredDocumentFlow.editorVisualLineWidthRatios")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral(
        "rows: structuredDocumentFlow.editorLogicalGutterRows")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral(
        "logicalCursorPosition: editorDisplayBackend.presentationProjection.logicalLengthForSourceText(")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral(
        "logicalLineCount: editorDisplayBackend.presentationProjection.logicalLineCount")));
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha\nBeta"
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

    QObject* geometryProbe = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatRenderedGeometryProbe"));
    QVERIFY(geometryProbe != nullptr);
    QObject* nativeCursor = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatNativeCursor"));
    QVERIFY(nativeCursor != nullptr);
    QObject* surfaceSelectionEditor =
        rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatSurfaceSelectionEditor"));
    QVERIFY(surfaceSelectionEditor != nullptr);
    QTRY_COMPARE(geometryProbe->property("text").toString(), QStringLiteral("Alpha\nBeta"));
    QVERIFY(!geometryProbe->property("text").toString().contains(QStringLiteral("<span")));
    QTRY_COMPARE(surfaceSelectionEditor->property("text").toString(), QStringLiteral("Alpha\nBeta"));
    QVERIFY(!surfaceSelectionEditor->property("text").toString().contains(QStringLiteral("<span")));
    inlineEditor->setProperty("cursorPosition", 5);
    QTRY_VERIFY(inlineEditor->property("nativeCursorVisible").toBool());
    QTRY_VERIFY(nativeCursor->property("visible").toBool());
    QVERIFY(nativeCursor->property("width").toReal() >= 1.0);
    inlineEditor->setProperty("cursorPosition", 6);
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), 6);

    inlineEditor->setProperty("showRenderedOutput", false);
    QTRY_VERIFY(inlineEditor->property("nativeCursorVisible").toBool());
    QTRY_VERIFY(nativeCursor->property("visible").toBool());
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha beta"
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
    QVERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha beta"
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
    QVERIFY(!surfaceSelectionEditor->property("enabled").toBool());

    inlineEditor->setProperty("cursorPosition", 0);
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), 0);

    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(76, 20));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());
    QTRY_VERIFY(inlineEditor->property("cursorPosition").toInt() > 0);
    QTRY_VERIFY(inlineEditor->property("nativeCursorVisible").toBool());
    QVERIFY(!inlineEditor->property("nativeSelectionActive").toBool());

    const int clickedCursorPosition = inlineEditor->property("cursorPosition").toInt();
    const int clickedSourceCursorPosition = inlineEditor->property("sourceCursorPosition").toInt();
    QVERIFY(clickedSourceCursorPosition > QStringLiteral("<bold>").size());
    QTest::keyClick(&window, Qt::Key_Right, Qt::ShiftModifier);
    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(inlineEditor->property("cursorPosition").toInt() > clickedCursorPosition);
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), clickedSourceCursorPosition);
    QVERIFY(inlineEditor->property("selectionEnd").toInt() > clickedSourceCursorPosition);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_backspaceDeletesVisibleCharacterBeforeRenderedCursor()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha Beta"
        renderedText: "<span style='font-weight:700'>Al<span style='font-style:italic'>pha</span></span><span style='font-style:italic'> Beta</span>"
        showRenderedOutput: true
        text: "<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"

        onTextEdited: function (sourceText) {
            editor.text = sourceText;
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatRenderedBackspaceHarness.qml")));
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

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreVisibleLogicalSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 5),
        Q_ARG(QVariant, 5)));
    QVERIFY(restoreResult.toBool());
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), 5);

    QTest::keyClick(&window, Qt::Key_Backspace);

    QTRY_COMPARE(
        inlineEditor->property("text").toString(),
        QStringLiteral("<bold>Al<italic>ph</italic></bold><italic> Beta</italic>"));
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), 4);
    QVERIFY(!inlineEditor->property("nativeSelectionActive").toBool());
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsNativeSurfaceLogicalAndMapsTypingToRaw()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            id: projection
            sourceText: editor.text
        }
        displayGeometryText: projection.logicalText
        renderedText: projection.logicalText
        showRenderedOutput: true
        text: "<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"

        onTextEdited: function (sourceText) {
            editor.text = sourceText;
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatLogicalSurfaceTypingHarness.qml")));
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
    QObject* nativeEditor = inlineEditor->property("editorItem").value<QObject*>();
    QVERIFY(nativeEditor != nullptr);
    QTRY_COMPARE(nativeEditor->property("text").toString(), QStringLiteral("Alpha Beta"));
    QVERIFY(!nativeEditor->property("text").toString().contains(QStringLiteral("<bold>")));
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreVisibleLogicalSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 5),
        Q_ARG(QVariant, 5)));
    QVERIFY(restoreResult.toBool());

    QTest::keyClick(&window, Qt::Key_X);
    QTRY_COMPARE(nativeEditor->property("text").toString(), QStringLiteral("Alphax Beta"));
    QTRY_COMPARE(
        inlineEditor->property("text").toString(),
        QStringLiteral("<bold>Al<italic>pha</italic></bold>x<italic> Beta</italic>"));

    QTest::keyClick(&window, Qt::Key_Backspace);
    QTRY_COMPARE(nativeEditor->property("text").toString(), QStringLiteral("Alpha Beta"));
    QTRY_COMPARE(
        inlineEditor->property("text").toString(),
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"));
}

void WhatSonCppRegressionTests::qmlStructuredDocumentFlow_holdsLogicalSurfaceWhileProjectionCatchesUp()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    objectName: "structuredDocumentFlowProjectionCatchupHarness"
    property string committedText: ""
    property string projectionBody: "<bold>Example</bold>"
    property string projectionHtml: "<span style='font-weight:700'>Example</span>"
    property string projectionLogicalText: "Example"
    property string sourceBody: "<bold>Example</bold>"
    width: 360
    height: 96

    function publishProjectionCatchup() {
        root.projectionLogicalText = "Examplex";
        root.projectionHtml = "<span style='font-weight:700'>Example</span>x";
        root.projectionBody = root.sourceBody;
    }

    EditorView.ContentsStructuredDocumentFlow {
        id: documentFlow
        objectName: "structuredDocumentFlowUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: documentFlow.projectionSourceText
        }
        editorSurfaceHtml: root.projectionHtml
        logicalText: root.projectionLogicalText
        projectionSourceText: root.projectionBody
        sourceText: root.sourceBody

        onSourceTextEdited: function (text) {
            root.committedText = text;
            root.sourceBody = text;
            root.projectionHtml = "";
            root.projectionLogicalText = "";
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/StructuredDocumentFlowProjectionCatchupHarness.qml")));
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

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("contentsStructuredDocumentInlineEditor"));
    QVERIFY(inlineEditor != nullptr);
    QObject* renderedOverlay = rootObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatRenderedOverlay"));
    QVERIFY(renderedOverlay != nullptr);
    QObject* nativeEditor = inlineEditor->property("editorItem").value<QObject*>();
    QVERIFY(nativeEditor != nullptr);
    QTRY_COMPARE(inlineEditor->property("displayGeometryText").toString(), QStringLiteral("Example"));
    QTRY_COMPARE(nativeEditor->property("text").toString(), QStringLiteral("Example"));
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(renderedOverlay->property("text").toString().contains(QStringLiteral("font-weight:700")));
    QVERIFY(!nativeEditor->property("text").toString().contains(QStringLiteral("<bold>")));
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));

    QVariant restoreResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreVisibleLogicalSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 7),
        Q_ARG(QVariant, 7)));
    QVERIFY(restoreResult.toBool());

    QTest::keyClick(&window, Qt::Key_X);

    QTRY_COMPARE(rootObject->property("committedText").toString(), QStringLiteral("<bold>Example</bold>x"));
    QTRY_COMPARE(nativeEditor->property("text").toString(), QStringLiteral("Examplex"));
    QVERIFY(!nativeEditor->property("text").toString().contains(QStringLiteral("<bold>")));
    QCOMPARE(inlineEditor->property("displayGeometryText").toString(), QStringLiteral("Example"));
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(renderedOverlay->property("text").toString().contains(QStringLiteral("font-weight:700")));
    QVERIFY(!renderedOverlay->property("text").toString().contains(QStringLiteral("Example</span>x")));

    QVERIFY(QMetaObject::invokeMethod(rootObject.get(), "publishProjectionCatchup"));
    QTRY_COMPARE(inlineEditor->property("displayGeometryText").toString(), QStringLiteral("Examplex"));
    QTRY_COMPARE(nativeEditor->property("text").toString(), QStringLiteral("Examplex"));
    QVERIFY(!nativeEditor->property("text").toString().contains(QStringLiteral("<bold>")));
    QTRY_VERIFY(renderedOverlay->property("text").toString().contains(QStringLiteral("Example</span>x")));
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha beta"
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
    QVERIFY(!surfaceSelectionEditor->property("enabled").toBool());

    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(40, 20));
    QTest::mouseMove(&window, QPoint(140, 20), 20);
    QTest::qWait(40);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(140, 20));

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(inlineEditor->property("selectionEnd").toInt() > inlineEditor->property("selectionStart").toInt());
    QVERIFY(!inlineEditor->property("selectedText").toString().isEmpty());
    QVERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
    QVERIFY(!inlineEditor->property("nativeCursorVisible").toBool());
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 160

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
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
    QVERIFY(!inlineEditor->property("nativeCursorVisible").toBool());

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
    QVERIFY(!inlineEditor->property("nativeCursorVisible").toBool());
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha beta"
        logicalCursorPosition: 0
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

    inlineEditor->setProperty("cursorPosition", 1);
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), 1);
    QTRY_VERIFY(inlineEditor->property("sourceCursorPosition").toInt() > inlineEditor->property("cursorPosition").toInt());

    const int closingTagStart = QStringLiteral("<bold>Alpha").size();
    const int closingTagEnd = closingTagStart + QStringLiteral("</bold>").size();
    Q_UNUSED(closingTagStart);
    inlineEditor->setProperty("cursorPosition", QStringLiteral("Alpha").size());
    QTRY_COMPARE(inlineEditor->property("cursorPosition").toInt(), QStringLiteral("Alpha").size());
    QTRY_COMPARE(inlineEditor->property("sourceCursorPosition").toInt(), closingTagEnd);
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 96
    height: 160

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 220
    height: 180

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
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

void WhatSonCppRegressionTests::qmlInlineFormatEditor_placesResourceGutterRowsAfterFrame()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    QTemporaryDir imageDirectory;
    QVERIFY(imageDirectory.isValid());
    const QString imagePath = imageDirectory.filePath(QStringLiteral("resource-frame.png"));
    QImage image(24, 24, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0x44, 0x88, 0xcc));
    QVERIFY(image.save(imagePath));

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QString imageUrl = QUrl::fromLocalFile(imagePath).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 260
    height: 260

    readonly property string resourceTag: "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/demo.wsresource\" id=\"demo\" />"
    readonly property int resourceStart: 0
    readonly property int resourceEnd: resourceTag.length

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "\uFFFC\nBeta"
        normalizedHtmlBlocks: [
            {
                "htmlBlockIsDisplayBlock": true,
                "htmlBlockObjectSource": "iiHtmlBlock",
                "renderDelegateType": "resource",
                "sourceStart": root.resourceStart,
                "sourceEnd": root.resourceEnd,
                "sourceText": root.resourceTag
            }
        ]
        documentBlocks: normalizedHtmlBlocks
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;line-height:0px;'><img src='%2' width='160' height='96' style='vertical-align:top;' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        resourceVisualBlocks: [{"height": 96, "imageSource": "%2", "renderable": true, "width": 160}]
        showRenderedOutput: true
        text: root.resourceTag + "\nBeta"
    }
}
)QML").arg(editorImportUrl, imageUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatResourceGutterHarness.qml")));
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
    window.resize(260, 260);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QTRY_COMPARE(inlineEditor->property("logicalGutterRows").toList().size(), 2);

    QCOMPARE(inlineEditor->property("lineNumberGeometryResourceBlockHeights").toList().first().toDouble(), 96.0);
    QVariantList geometryRows = inlineEditor->property("lineNumberGeometryRows").toList();
    QCOMPARE(geometryRows.size(), 2);
    QCOMPARE(geometryRows.at(0).toMap().value(QStringLiteral("height")).toDouble(), 96.0);
    QTRY_VERIFY(qAbs(
                    inlineEditor->property("lineNumberGeometryRows")
                        .toList()
                        .at(1)
                        .toMap()
                        .value(QStringLiteral("y"))
                        .toDouble()
                    - 96.0)
                <= 1.0);
    geometryRows = inlineEditor->property("lineNumberGeometryRows").toList();
    QCOMPARE(geometryRows.at(1).toMap().value(QStringLiteral("y")).toDouble(), 96.0);
    QTRY_VERIFY(qAbs(
                    (inlineEditor->property("logicalGutterRows").toList()
                         .at(1)
                         .toMap()
                         .value(QStringLiteral("y"))
                         .toDouble()
                     - inlineEditor->property("logicalGutterRows").toList()
                           .at(0)
                           .toMap()
                           .value(QStringLiteral("y"))
                           .toDouble())
                    - 96.0)
                <= 1.0);
    QVariantList logicalGutterRows = inlineEditor->property("logicalGutterRows").toList();
    QVariantMap resourceRow = logicalGutterRows.at(0).toMap();
    QVariantMap betaRow = logicalGutterRows.at(1).toMap();
    logicalGutterRows = inlineEditor->property("logicalGutterRows").toList();
    resourceRow = logicalGutterRows.at(0).toMap();
    betaRow = logicalGutterRows.at(1).toMap();
    QCOMPARE(resourceRow.value(QStringLiteral("number")).toInt(), 1);
    QVERIFY(resourceRow.value(QStringLiteral("resourceRange")).toBool());
    QCOMPARE(betaRow.value(QStringLiteral("number")).toInt(), 2);
    const double resourceY = resourceRow.value(QStringLiteral("y")).toDouble();
    const double betaY = betaRow.value(QStringLiteral("y")).toDouble();
    const double renderedFrameHeight = 96.0;
    QVERIFY2(
        qAbs((betaY - resourceY) - renderedFrameHeight) <= 1.0,
        qPrintable(QStringLiteral("Second gutter row must match rendered resource bottom: betaY=%1 resourceY=%2 frameHeight=%3")
                       .arg(betaY)
                       .arg(resourceY)
                       .arg(renderedFrameHeight)));
    QVERIFY(resourceRow.value(QStringLiteral("height")).toDouble() < betaY - resourceY);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_separatesBlankGutterRowAfterResourceFrame()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    QTemporaryDir imageDirectory;
    QVERIFY(imageDirectory.isValid());
    const QString imagePath = imageDirectory.filePath(QStringLiteral("resource-frame.png"));
    QImage image(24, 24, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0x44, 0x88, 0xcc));
    QVERIFY(image.save(imagePath));

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QString imageUrl = QUrl::fromLocalFile(imagePath).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 260
    height: 260

    readonly property string resourceTag: "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/demo.wsresource\" id=\"demo\" />"
    readonly property int resourceStart: 0
    readonly property int resourceEnd: resourceTag.length

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "\uFFFC\nBeta\n"
        normalizedHtmlBlocks: [
            {
                "htmlBlockIsDisplayBlock": true,
                "htmlBlockObjectSource": "iiHtmlBlock",
                "renderDelegateType": "resource",
                "sourceStart": root.resourceStart,
                "sourceEnd": root.resourceEnd,
                "sourceText": root.resourceTag
            }
        ]
        documentBlocks: normalizedHtmlBlocks
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;line-height:0px;'><img src='%2' width='160' height='96' style='vertical-align:top;' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p><p style='margin-top:0px;margin-bottom:0px;'>&nbsp;</p>"
        resourceVisualBlocks: [{"height": 96, "imageSource": "%2", "renderable": true, "width": 160}]
        showRenderedOutput: true
        text: root.resourceTag + "\nBeta\n"
    }
}
)QML").arg(editorImportUrl, imageUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatResourceBlankGutterHarness.qml")));
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
    window.resize(260, 260);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QTRY_COMPARE(inlineEditor->property("logicalGutterRows").toList().size(), 3);

    const QVariantList logicalGutterRows = inlineEditor->property("logicalGutterRows").toList();
    const QVariantMap resourceRow = logicalGutterRows.at(0).toMap();
    const QVariantMap betaRow = logicalGutterRows.at(1).toMap();
    const QVariantMap blankRow = logicalGutterRows.at(2).toMap();
    QCOMPARE(resourceRow.value(QStringLiteral("number")).toInt(), 1);
    QCOMPARE(betaRow.value(QStringLiteral("number")).toInt(), 2);
    QCOMPARE(blankRow.value(QStringLiteral("number")).toInt(), 3);
    const double resourceY = resourceRow.value(QStringLiteral("y")).toDouble();
    const double betaY = betaRow.value(QStringLiteral("y")).toDouble();
    const double blankY = blankRow.value(QStringLiteral("y")).toDouble();
    QVERIFY2(
        qAbs((betaY - resourceY) - 96.0) <= 1.0,
        qPrintable(QStringLiteral("Text row after resource must start at frame bottom: betaY=%1 resourceY=%2")
                       .arg(betaY)
                       .arg(resourceY)));
    QVERIFY2(
        blankY > betaY,
        qPrintable(QStringLiteral("Blank gutter row after resource text must not overlap line 2: blankY=%1 betaY=%2")
                       .arg(blankY)
                       .arg(betaY)));
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 220

    EditorView.ContentsStructuredDocumentFlow {
        id: documentFlow
        objectName: "structuredDocumentFlowUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: documentFlow.sourceText
        }
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
    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("contentsStructuredDocumentInlineEditor"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_VERIFY(documentFlow->property("editorContentHeight").toReal() > 0.0);
    QTRY_VERIFY(documentFlow->property("editorContentHeight").toReal() < 200.0);
    QTRY_VERIFY(inlineEditor->property("editorBottomInset").toReal() > 0.0);
    QTRY_VERIFY(inlineEditor->property("displayContentHeight").toReal()
                > inlineEditor->property("displayBodyHeight").toReal());
    QCOMPARE(
        qRound(inlineEditor->property("displayBodyHeight").toReal()),
        qRound(inlineEditor->property("displayTextContentHeight").toReal()));
    QCOMPARE(
        qRound(inlineEditor->property("displayContentHeight").toReal()
               - inlineEditor->property("displayBodyHeight").toReal()),
        qRound(inlineEditor->property("editorBottomInset").toReal()));

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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 160

    EditorView.ContentsStructuredDocumentFlow {
        id: documentFlow
        objectName: "structuredDocumentFlowUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: documentFlow.sourceText
        }
        editorSurfaceHtml: "<span style='font-weight:700'>Alpha beta</span>"
        logicalText: "Alpha beta"
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

    QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(76, 20));
    QTRY_VERIFY(documentFlow->property("editorCursorPosition").toInt() > QStringLiteral("<bold>").size());
    QTRY_VERIFY(inlineEditor->property("nativeCursorVisible").toBool());
}

void WhatSonCppRegressionTests::qmlStructuredDocumentFlow_preservesRenderedPointerDragSelectionInsideFlickable()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 160

    Flickable {
        id: viewport
        objectName: "documentViewportUnderTest"
        anchors.fill: parent
        contentHeight: contentItemWrapper.height
        contentWidth: width
        interactive: !documentFlow.editorRenderedOverlayVisible && contentHeight > height

        Item {
            id: contentItemWrapper
            width: viewport.width
            height: 320

            EditorView.ContentsStructuredDocumentFlow {
                id: documentFlow
                objectName: "structuredDocumentFlowUnderTest"
                width: parent.width
                height: parent.height
                coordinateMapper: ContentsEditorPresentationProjection {
                    sourceText: documentFlow.sourceText
                }
                editorSurfaceHtml: "<span style='font-weight:700'>Alpha beta gamma delta</span>"
                logicalText: "Alpha beta gamma delta"
                sourceText: "<bold>Alpha beta gamma delta</bold>"
            }
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/StructuredDocumentFlowFlickableDragSelectionHarness.qml")));
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
    QObject* viewport = rootObject->findChild<QObject*>(QStringLiteral("documentViewportUnderTest"));
    QVERIFY(viewport != nullptr);
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
    QVERIFY(!viewport->property("interactive").toBool());
    QCOMPARE(viewport->property("contentY").toReal(), 0.0);

    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(40, 20));
    QTest::mouseMove(&window, QPoint(180, 20), 20);
    QTest::qWait(40);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(180, 20));

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(inlineEditor->property("selectionEnd").toInt() > inlineEditor->property("selectionStart").toInt());
    QVERIFY(!inlineEditor->property("selectedText").toString().isEmpty());
    QVERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
    QCOMPARE(viewport->property("contentY").toReal(), 0.0);
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
import WhatSon.App.Internal 1.0
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
import WhatSon.App.Internal 1.0
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
        coordinateMapper: ContentsEditorPresentationProjection {
            id: projection
            sourceText: documentFlow.sourceText
        }
        editorSurfaceHtml: projection.logicalText
        logicalText: projection.logicalText
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

    QTRY_VERIFY_WITH_TIMEOUT(
        rootObject->property("committedText").toString().startsWith(QStringLiteral("Meta agenda\n<agenda date=\"")),
        5000);
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
import WhatSon.App.Internal 1.0
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
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha\n\uFFFC\nBeta"
        normalizedHtmlBlocks: [{
            "htmlBlockIsDisplayBlock": true,
            "htmlBlockObjectSource": "iiHtmlBlock",
            "renderDelegateType": "resource",
            "sourceStart": root.resourceStart,
            "sourceEnd": root.resourceEnd
        }]
        documentBlocks: normalizedHtmlBlocks
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'><img src='' width='120' height='60' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        resourceVisualBlocks: [{"height": 60, "renderable": false, "width": 120}]
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
    QVERIFY(inlineEditor->property("selectionStart").toInt() <= rootObject->property("resourceStart").toInt());
    QVERIFY(inlineEditor->property("selectionEnd").toInt() >= rootObject->property("resourceEnd").toInt());
    QVERIFY(!inlineEditor->property("selectedText").toString().contains(QStringLiteral("<resource type=\"image\"")));
    QTRY_COMPARE(
        renderedOverlay->property("selectionEnd").toInt(),
        renderedOverlay->property("selectionStart").toInt());
    QVERIFY(!inlineEditor->property("atomicResourceSelectionRects").isValid());
    QTRY_VERIFY(inlineEditor->property("renderedOverlayVisible").toBool());
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_snapsPointerSelectionInsideResourceFrameToAtomicBlock()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    readonly property string resourceTag: "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/demo.wsresource\" id=\"demo\" />"
    readonly property int resourceStart: 6
    readonly property int resourceEnd: resourceStart + resourceTag.length
    width: 360
    height: 180

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha\n\uFFFC\nBeta"
        normalizedHtmlBlocks: [{
            "htmlBlockIsDisplayBlock": true,
            "htmlBlockObjectSource": "iiHtmlBlock",
            "renderDelegateType": "resource",
            "sourceStart": root.resourceStart,
            "sourceEnd": root.resourceEnd
        }]
        documentBlocks: normalizedHtmlBlocks
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'><img src='' width='300' height='72' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        resourceVisualBlocks: [{"height": 72, "renderable": false, "width": 300}]
        showRenderedOutput: true
        text: "Alpha\n" + root.resourceTag + "\nBeta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatResourcePointerHarness.qml")));
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
    QTRY_COMPARE(inlineEditor->property("logicalGutterRows").toList().size(), 3);

    QVariant resourceHit;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "atomicResourceHitAtPoint",
        Q_RETURN_ARG(QVariant, resourceHit),
        Q_ARG(QVariant, 40),
        Q_ARG(QVariant, 48)));
    QVERIFY(resourceHit.toMap().value(QStringLiteral("hit")).toBool());

    QVariant beginResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "beginVisiblePointerSelection",
        Q_RETURN_ARG(QVariant, beginResult),
        Q_ARG(QVariant, 40),
        Q_ARG(QVariant, 48)));
    QVERIFY(beginResult.toBool());

    QTRY_VERIFY(inlineEditor->property("nativeSelectionActive").toBool());
    QCOMPARE(inlineEditor->property("selectionStart").toInt(), rootObject->property("resourceStart").toInt());
    QCOMPARE(inlineEditor->property("selectionEnd").toInt(), rootObject->property("resourceEnd").toInt());
    const QString selectedText = inlineEditor->property("selectedText").toString();
    QVERIFY(!selectedText.contains(QStringLiteral("Alpha")));
    QVERIFY(!selectedText.contains(QStringLiteral("Beta")));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsCursorOutOfAtomicResourceFrame()
{
    registerInlineFormatEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/contents/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    readonly property string resourceTag: "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/demo.wsresource\" id=\"demo\" />"
    readonly property int resourceStart: 6
    readonly property int resourceEnd: resourceStart + resourceTag.length
    width: 360
    height: 180

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha\n\uFFFC\nBeta"
        normalizedHtmlBlocks: [{
            "htmlBlockIsDisplayBlock": true,
            "htmlBlockObjectSource": "iiHtmlBlock",
            "renderDelegateType": "resource",
            "sourceStart": root.resourceStart,
            "sourceEnd": root.resourceEnd
        }]
        documentBlocks: normalizedHtmlBlocks
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'><img src='' width='300' height='72' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        resourceVisualBlocks: [{"height": 72, "renderable": false, "width": 300}]
        showRenderedOutput: true
        text: "Alpha\n" + root.resourceTag + "\nBeta"
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatResourceCursorHarness.qml")));
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
    QTRY_COMPARE(inlineEditor->property("logicalGutterRows").toList().size(), 3);
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    inlineEditor->setProperty("previousRawCursorPosition", rootObject->property("resourceStart").toInt() - 1);
    QVariant setCursorResult;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "setCursorPositionPreservingNativeInput",
        Q_RETURN_ARG(QVariant, setCursorResult),
        Q_ARG(QVariant, 6)));
    QTRY_COMPARE(
        inlineEditor->property("sourceCursorPosition").toInt(),
        rootObject->property("resourceEnd").toInt() + 1);
    QVERIFY(!inlineEditor->property("atomicResourceCursorActive").toBool());
    QVERIFY(inlineEditor->property("nativeCursorVisible").toBool());

    inlineEditor->setProperty("previousRawCursorPosition", rootObject->property("resourceEnd").toInt() + 1);
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "setCursorPositionPreservingNativeInput",
        Q_RETURN_ARG(QVariant, setCursorResult),
        Q_ARG(QVariant, 7)));
    QTRY_COMPARE(
        inlineEditor->property("sourceCursorPosition").toInt(),
        rootObject->property("resourceStart").toInt() - 1);
    QVERIFY(!inlineEditor->property("atomicResourceCursorActive").toBool());
    QVERIFY(inlineEditor->property("nativeCursorVisible").toBool());
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
import WhatSon.App.Internal 1.0
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
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Next"
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

    QTRY_VERIFY(!inlineEditor->property("nativeSelectionActive").toBool());
    QVERIFY(!inlineEditor->property("selectedText").toString().contains(QStringLiteral("<highlight>")));
    QTRY_VERIFY(!inlineEditor->property("nativeSelectionContainsVisibleLogicalContent").toBool());
    QTRY_VERIFY(!inlineEditor->property("nativeSelectionPaintVisible").toBool());
    QTRY_VERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
    QTRY_VERIFY(
        renderedOverlay->property("selectionEnd").toInt()
        == renderedOverlay->property("selectionStart").toInt());
    QVERIFY(!inlineEditor->property("atomicResourceSelectionRects").isValid());

    QVERIFY(QMetaObject::invokeMethod(
        inlineEditor,
        "restoreSelectionRange",
        Q_RETURN_ARG(QVariant, restoreResult),
        Q_ARG(QVariant, 1),
        Q_ARG(QVariant, emptyFormattingTagsLength - 1),
        Q_ARG(QVariant, emptyFormattingTagsLength - 1)));
    QVERIFY(restoreResult.toBool());

    QTRY_VERIFY(!inlineEditor->property("nativeSelectionActive").toBool());
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
    QVERIFY(!inlineEditor->property("renderedSelectionActive").toBool());
    QTRY_COMPARE(
        renderedOverlay->property("selectionEnd").toInt(),
        renderedOverlay->property("selectionStart").toInt());
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
import WhatSon.App.Internal 1.0
import "%1" as EditorView

Item {
    id: root
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        renderedText: "<b>Alpha</b> beta"
        showRenderedOutput: true
        text: "Alpha beta"

        onTextEdited: function (sourceText) {
            editor.text = sourceText;
        }
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
import WhatSon.App.Internal 1.0
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
        coordinateMapper: ContentsEditorPresentationProjection {
            sourceText: editor.text
        }
        displayGeometryText: "Alpha\n\uFFFC\nBeta"
        normalizedHtmlBlocks: [{
            "htmlBlockIsDisplayBlock": true,
            "htmlBlockObjectSource": "iiHtmlBlock",
            "renderDelegateType": "resource",
            "sourceStart": root.resourceStart,
            "sourceEnd": root.resourceEnd
        }]
        documentBlocks: normalizedHtmlBlocks
        renderedText: "<p style='margin-top:0px;margin-bottom:0px;'>Alpha</p><p style='margin-top:0px;margin-bottom:0px;'><img src='' width='120' height='60' /></p><p style='margin-top:0px;margin-bottom:0px;'>Beta</p>"
        resourceVisualBlocks: [{"height": 60, "renderable": false, "width": 120}]
        showRenderedOutput: true
        text: "Alpha\n" + root.resourceTag + "\nBeta"

        onTextEdited: function (sourceText) {
            editor.text = sourceText;
        }
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
    const QString inlineEditorControllerHelperPath =
        qmlInlineFormatEditorRepositoryRootPath()
        + QStringLiteral("/src/app/qml/view/contents/editor/ContentsInlineFormatEditorController.qml");
    const QString surfaceGuardHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsEditorSurfaceGuardController.hpp"));
    const QString resourceImportControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.hpp"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerHeader.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY2(
        !QFileInfo::exists(inlineEditorControllerHelperPath),
        "ContentsInlineFormatEditorController.qml must not exist; input controller state belongs in C++.");
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
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("QStringLiteral(\"logicalCursorPosition\")")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("QStringLiteral(\"logicalSelectionStart\")")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("QStringLiteral(\"logicalSelectionEnd\")")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool inputMethodComposing: textInput.inputMethodComposing")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property string preeditText: String(textInput.editorItem.preeditText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function programmaticTextSyncPolicy(nextText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("return inlineEditorController.programmaticTextSyncPolicy(nextText);")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool preferNativeInputHandling: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("inlineEditorController.setProgrammaticText(resolvedText)")));
    QVERIFY(inlineEditorControllerHeader.contains(QStringLiteral("ContentsEditorInputPolicyAdapter")));
    QVERIFY(!inlineEditorControllerHeader.contains(QStringLiteral("eventFilter")));
    QVERIFY(inlineEditorControllerHeader.contains(QStringLiteral("m_localSelectionInteractionSinceFocus")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("nativeSelectionActive() const")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("invokeRenderedBackspaceMutation")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("applyRenderedBackspaceMutation")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("installEventFilter")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("removeEventFilter")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("m_localSelectionInteractionSinceFocus || nativeSelectionActive()")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("ContentsInlineFormatEditorController::programmaticTextSyncPolicy")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("invokeHelper")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("helperSourceUrl")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("ContentsInlineFormatEditorController.qml")));
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
