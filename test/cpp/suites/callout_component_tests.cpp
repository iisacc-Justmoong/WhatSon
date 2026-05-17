#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

void WhatSonCppRegressionTests::calloutComponent_rendersFigmaCalloutBlock()
{
    WhatSon::EditorComponent::CalloutDescriptor descriptor;
    descriptor.sourceText = QStringLiteral("<callout>This is callout text</callout>");
    descriptor.contentHtml = QStringLiteral("This is callout text");

    const QString html = WhatSon::EditorComponent::Callout::renderHtml(descriptor);

    QVERIFY(html.contains(QStringLiteral("<!--whatson-callout-source:")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-callout\"")));
    QVERIFY(html.contains(QStringLiteral("data-figma-node-id=\"280:7897\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-design-width=\"295\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-width-mode=\"fill\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-height-mode=\"hug-contents\"")));
    QVERIFY(html.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(!html.contains(QStringLiteral("height=\"")));
    QVERIFY(html.contains(QStringLiteral("background-color:#262728")));
    QVERIFY(html.contains(QStringLiteral("padding:16px 4px")));
    QVERIFY(!html.contains(QStringLiteral("padding:4px;")));
    QVERIFY(html.contains(QStringLiteral("height:auto")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-design-height")));
    QVERIFY(!html.contains(QStringLiteral("height:22px")));
    QVERIFY(!html.contains(QStringLiteral("<table")));
    QVERIFY(!html.contains(QStringLiteral("<td")));
    QVERIFY(!html.contains(QStringLiteral("class=\"whatson-callout-bar\"")));
    QVERIFY(!html.contains(QStringLiteral("class=\"whatson-callout-gap\"")));
    QVERIFY(!html.contains(QStringLiteral("&nbsp;")));
    QVERIFY(html.contains(QStringLiteral("border-left:3px solid #d9d9d9")));
    QVERIFY(html.contains(QStringLiteral("padding-left:12px")));
    QVERIFY(html.contains(QStringLiteral("data-callout-content=\"true\"")));
    QVERIFY(html.contains(QStringLiteral("font-family:Pretendard")));
    QVERIFY(html.contains(QStringLiteral("font-size:12px")));
    QVERIFY(html.contains(QStringLiteral("line-height:12px")));
    QVERIFY(html.contains(QStringLiteral("This is callout text")));

    QTextDocument editorDocument;
    editorDocument.setHtml(html);
    QCOMPARE(editorDocument.toPlainText().trimmed(), QStringLiteral("This is callout text"));
    QVERIFY(!editorDocument.toPlainText().contains(QChar::Nbsp));
}
