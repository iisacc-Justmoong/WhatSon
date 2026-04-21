#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlResourceEditorView_staysTransparentAndViewerOnly()
{
    const QString resourceEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceEditorView.qml"));

    QVERIFY(!resourceEditorSource.isEmpty());
    QVERIFY(resourceEditorSource.contains(QStringLiteral("Item {")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("property color displayColor: \"transparent\"")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("ContentsResourceViewer {")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("anchors.fill: parent")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("visible: resourceEditor.hasResourceSelection && resourceBitmapState.bitmapRenderable")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("LV.Theme.panelBackground03")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("#CC0F141A")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("No Resource Selected")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("Preview Unavailable")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("The dedicated resource editor currently previews image resources first.")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("The resource editor now owns direct resource preview for the Resources hierarchy.")));
}
