#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QStringList>

namespace
{
    QString textEditorQmlPath()
    {
        return QStringLiteral("src/app/qml/view/contents/TextEditor.qml");
    }

    QString contentViewLayoutQmlPath()
    {
        return QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml");
    }

    void verifyNoTokens(const QString& source, const QStringList& tokens)
    {
        for (const QString& token : tokens)
        {
            QVERIFY2(!source.contains(token), qPrintable(QStringLiteral("Unexpected token: %1").arg(token)));
        }
    }
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_keepsLvrsTextEditorSurface()
{
    const QString textEditorSource = readUtf8SourceFile(textEditorQmlPath());
    const QString contentViewLayoutSource = readUtf8SourceFile(contentViewLayoutQmlPath());

    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!contentViewLayoutSource.isEmpty());
    QVERIFY(textEditorSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property bool editorReadOnly: false")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property string noteBodyFilePath: \"\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readOnly: textEditor.editorReadOnly || textEditor.noteBodyFilePath.trim().length === 0")));
    QVERIFY(textEditorSource.contains(QStringLiteral("preferNativeGestures: LV.Theme.mobileTarget")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorViewportHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorViewportContentHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorViewportWidth")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property real editorBottomViewportPaddingRatio: 0.5")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorBottomViewportPadding")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorMeasuredContentHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property int editorRenderedLineCount")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorVisualLineHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property int editorLineMetricsRevision: 0")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorLineMetricsFor(lineIndex)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function scrollEditorViewportTo(contentY)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property: \"bottomPadding\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("value: textEditor.editorBottomViewportPadding")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.positionToRectangle")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.editorItem")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineCount: contentsTextEditor.editorRenderedLineCount")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("lineCount: contentViewLayout.editorParsedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("fallbackLineHeight: contentsTextEditor.editorVisualLineHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineMetricProvider: contentsTextEditor.editorLineMetricsFor")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineMetricsRevision: contentsTextEditor.editorLineMetricsRevision")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("documentText: contentsTextEditor.editorDocumentText")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceContentHeight: contentsTextEditor.editorViewportContentHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceViewportHeight: contentsTextEditor.editorViewportHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("scrollTarget: contentsTextEditor.scrollEditorViewportTo")));
    QVERIFY(textEditorSource.contains(QStringLiteral("showScrollBar: false")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("import \"../contents\" as ContentsView")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteEditorSession")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteBodyFilePath: contentViewLayout.editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("persistEditorFile(path)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Gutter {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.TextEditor {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Minimap {")));

    verifyNoTokens(textEditorSource + contentViewLayoutSource, {
        QStringLiteral("LV.CodeEditor"),
        QStringLiteral("TextEdit {"),
        QStringLiteral("ContentsStructuredDocumentFlow"),
        QStringLiteral("ContentsLineNumberRail"),
        QStringLiteral("ContentsMinimapLayoutMetrics"),
        QStringLiteral("ContentsResourceEditorView"),
        QStringLiteral("ContentsEditorDisplayBackend"),
        QStringLiteral("ContentsPagePrintLayoutRenderer"),
    });
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_excludesSnapshotProjectionPersistence()
{
    const QString textEditorSource = readUtf8SourceFile(textEditorQmlPath());
    const QString contentViewLayoutSource = readUtf8SourceFile(contentViewLayoutQmlPath());

    verifyNoTokens(textEditorSource + contentViewLayoutSource, {
        QStringLiteral("sourceText"),
        QStringLiteral("renderedText"),
        QStringLiteral("projection"),
        QStringLiteral("normalizedHtmlBlocks"),
        QStringLiteral("documentBlocks"),
        QStringLiteral("resourceVisualBlocks"),
        QStringLiteral("coordinateMapper"),
        QStringLiteral("commitEditedSourceText"),
        QStringLiteral("editorTextEdited"),
        QStringLiteral("persistence"),
    });
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_keepsNativeSurfaceOnly()
{
    const QString textEditorSource = readUtf8SourceFile(textEditorQmlPath());

    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    verifyNoTokens(textEditorSource, {
        QStringLiteral("largeDocumentNativeSurface"),
        QStringLiteral("renderedOverlay"),
        QStringLiteral("displayGeometryText"),
        QStringLiteral("ContentsEditorGeometryProvider"),
        QStringLiteral("ContentsEditorVisualLineMetrics"),
        QStringLiteral("ContentsLineNumberRailMetrics"),
    });
}

void WhatSonCppRegressionTests::qmlContentViewLayout_wiresEditorFormatShortcutsOutsideTextEditor()
{
    const QString textEditorSource = readUtf8SourceFile(textEditorQmlPath());
    const QString contentViewLayoutSource = readUtf8SourceFile(contentViewLayoutQmlPath());

    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!contentViewLayoutSource.isEmpty());
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property int editorSelectionStart")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property int editorSelectionLength")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Shortcut {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("insertFormatTagIntoSource")));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function applyEditorFormatTag(tagName)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("insertFormatTagIntoSource")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor.editorSelectionStart")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor.editorSelectionLength")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("formatResult.editorDocumentText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function restoreEditorCursorPosition(nextCursorPosition)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("Qt.callLater(function ()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.forceEditorFocus();")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag(\"bold\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: StandardKey.Bold")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag(\"italic\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: StandardKey.Italic")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag(\"underline\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: StandardKey.Underline")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag(\"strikethrough\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+X\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: \"Meta+Shift+X\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag(\"highlight\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+E\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: \"Meta+Shift+E\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag(\"break\")")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+B\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: \"Meta+Shift+B\"")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+H\"")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("sequence: \"Meta+Shift+H\"")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("sequence: \"Ctrl+Shift+Return\"")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("sequence: \"Meta+Shift+Return\"")));
}

void WhatSonCppRegressionTests::qmlContentViewLayout_opensEditorFormatContextMenuForSelection()
{
    const QString textEditorSource = readUtf8SourceFile(textEditorQmlPath());
    const QString contentViewLayoutSource = readUtf8SourceFile(contentViewLayoutQmlPath());

    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!contentViewLayoutSource.isEmpty());
    QVERIFY(!textEditorSource.contains(QStringLiteral("LV.ContextMenu")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Qt.RightButton")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("import QtQuick.Controls as Controls")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property bool editorFormatContextMenuAvailable")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor.editorSelectionLength > 0")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property var editorFormatContextMenuItems")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("\"eventName\": \"editor.format.bold\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("\"eventName\": \"editor.format.italic\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("\"eventName\": \"editor.format.underline\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("\"eventName\": \"editor.format.strikethrough\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("\"eventName\": \"editor.format.highlight\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function openEditorFormatContextMenuFromPointer")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("function handleEditorFormatContextMenuTrigger(eventName)")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("editorFormatContextMenu.openFor")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("TapHandler {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("acceptedButtons: Qt.RightButton")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("enabled: contentViewLayout.editorFormatContextMenuAvailable")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("openEditorFormatContextMenuFromPointer(")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentsTextEditor,")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("LV.ContextMenu {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("id: editorFormatContextMenu")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("items: contentViewLayout.editorFormatContextMenuItems")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("onItemEventTriggered: function (eventName")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentViewLayout.handleEditorFormatContextMenuTrigger(eventName)")));
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_keepsKeyboardSelectionAndOsImeNative()
{
    const QString textEditorSource = readUtf8SourceFile(textEditorQmlPath());

    QVERIFY(textEditorSource.contains(QStringLiteral("preferNativeGestures: LV.Theme.mobileTarget")));
    verifyNoTokens(textEditorSource, {
        QStringLiteral("Keys.onPressed"),
        QStringLiteral("Keys.onReleased"),
        QStringLiteral("Shortcut {"),
        QStringLiteral("Qt.inputMethod"),
        QStringLiteral("InputMethod."),
        QStringLiteral("ContentsInlineFormatEditorController"),
        QStringLiteral("ContentsWysiwygEditorPolicy"),
        QStringLiteral("tagManagement"),
    });
}
