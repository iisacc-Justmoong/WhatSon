#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContentsTextEditor_keepsLvrsTextEditorSurface()
{
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));

    QVERIFY(textEditorSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("TextEdit {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("RichText")));
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_excludesSnapshotProjectionPersistence()
{
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(!textEditorSource.contains(QStringLiteral("snapshot")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("projection")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("projection")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("persistEditor")));
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_keepsNativeSurfaceOnly()
{
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));

    QVERIFY(textEditorSource.contains(QStringLiteral("selectByMouse: true")));
    QVERIFY(textEditorSource.contains(QStringLiteral("mouseSelectionMode: TextEdit.SelectCharacters")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("TapHandler")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onPressed")));
}

void WhatSonCppRegressionTests::qmlContentViewLayout_wiresEditorFormatShortcutsOutsideTextEditor()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("Shortcut {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("applyEditorFormatTag")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("insertFormatTagIntoSource")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorFormatContextMenu")));
}

void WhatSonCppRegressionTests::qmlContentViewLayout_opensEditorFormatContextMenuForSelection()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("LV.ContextMenu")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("openEditorFormatContextMenuFromPointer")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorSelectedText")));
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_keepsKeyboardSelectionAndOsImeNative()
{
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));

    QVERIFY(!textEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onReturnPressed")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onEnterPressed")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("preventDefault")));
}
