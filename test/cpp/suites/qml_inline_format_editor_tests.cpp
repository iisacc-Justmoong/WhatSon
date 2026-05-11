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
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorVisualLineHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property int editorLineMetricsRevision: 0")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function editorLineMetricsFor(lineIndex)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function scrollEditorViewportTo(contentY)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.positionToRectangle")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.editorItem")));
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
