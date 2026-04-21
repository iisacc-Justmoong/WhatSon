#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlEditors_routeRenderedHyperlinksToExternalBrowser()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onLinkActivated: function (link)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.openUrlExternally(link);")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onLinkActivated: function (link)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("Qt.openUrlExternally(link);")));
}
