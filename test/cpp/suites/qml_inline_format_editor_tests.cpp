#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsHiddenKeyboardTouchesScrollFirstOnMobile()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(
        QStringLiteral("readonly property bool nativeTouchScrollGuardActive: control.preferNativeInputHandling")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("enabled: control.nativeTouchScrollGuardActive")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("preventStealing: false")));
    QVERIFY(inlineEditorSource.contains(
        QStringLiteral("control.activateInputAtPoint(Qt.point(mouse.x, mouse.y));")));
    QVERIFY(inlineEditorSource.contains(
        QStringLiteral("enabled: control.preferNativeInputHandling\n                             && !control.nativeTouchScrollGuardActive")));
    QVERIFY(inlineEditorSource.contains(
        QStringLiteral("grabPermissions: PointerHandler.ApprovesTakeOverByAnything")));
}
