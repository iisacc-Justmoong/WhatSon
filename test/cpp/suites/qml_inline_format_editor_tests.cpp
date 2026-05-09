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
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: \"\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("preferNativeGestures: true")));
    QVERIFY(textEditorSource.contains(QStringLiteral("showScrollBar: false")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("import \"../contents\" as ContentsView")));
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

    QVERIFY(textEditorSource.contains(QStringLiteral("preferNativeGestures: true")));
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
