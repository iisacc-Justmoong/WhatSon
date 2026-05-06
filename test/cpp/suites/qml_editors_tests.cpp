#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlEditors_routeRenderedHyperlinksToExternalBrowser()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onLinkActivated: function (link)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.openUrlExternally(link);")));
    QVERIFY(!QFileInfo::exists(QStringLiteral(
        "src/app/qml/view/contents/editor/ContentsDisplaySurfaceHost.qml")));
}
