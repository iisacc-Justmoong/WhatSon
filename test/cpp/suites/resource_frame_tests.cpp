#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

void WhatSonCppRegressionTests::resourceFrame_rendersFigmaImageChrome()
{
    QTemporaryDir imageDirectory;
    QVERIFY(imageDirectory.isValid());
    const QString imagePath = QDir(imageDirectory.path()).filePath(QStringLiteral("capture.png"));
    QImage sourceImage(QSize(1600, 900), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(18, 110, 190, 255));
    QVERIFY(sourceImage.save(imagePath));

    WhatSon::EditorComponent::ResourceFrameDescriptor descriptor;
    descriptor.sourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    descriptor.resourcePath = QStringLiteral("Workspace.wsresources/capture.wsresource");
    descriptor.resourceId = QStringLiteral("capture");
    descriptor.type = QStringLiteral("image");
    descriptor.format = QStringLiteral(".png");
    descriptor.resolvedAssetPath = imagePath;

    const QString html = WhatSon::EditorComponent::ResourceFrame::renderHtml(descriptor);

    QVERIFY(html.contains(QStringLiteral("<!--whatson-resource-source:")));
    QVERIFY(html.startsWith(QStringLiteral("<!--whatson-resource-source:")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-resource-frame\"")));
    QVERIFY(!html.contains(QStringLiteral("<div class=\"whatson-resource-frame\"")));
    QVERIFY(!html.contains(QStringLiteral("data-whatson-component=\"resource-frame\"")));
    QVERIFY(!html.contains(QStringLiteral("<table")));
    QVERIFY(html.count(QStringLiteral("<img")) == 1);
    QVERIFY(html.contains(QStringLiteral("width:100%")));
    QVERIFY(html.contains(QStringLiteral("max-width:100%")));
    QVERIFY(html.contains(QStringLiteral("data-figma-node-id=\"292:50\"")));
    QVERIFY(html.contains(QStringLiteral("data-resource-type-label=\"Image\"")));
    QVERIFY(html.contains(QStringLiteral("data-resource-file-name=\"capture.wsresource\"")));
    QVERIFY(!html.contains(QStringLiteral("resourceHeader")));
    QVERIFY(!html.contains(QStringLiteral("resourceToolbar")));
    QVERIFY(!html.contains(QStringLiteral("whatson-resource-more")));
    QVERIFY(!html.contains(QStringLiteral("whatson-resource-type-display")));
    QVERIFY(!html.contains(QStringLiteral("whatson-resource-filename-display")));
    QVERIFY(!html.contains(QStringLiteral("data-display-role=\"resource-type\"")));
    QVERIFY(!html.contains(QStringLiteral("data-display-role=\"resource-file-name\"")));
    QVERIFY(!html.contains(QStringLiteral(" width=\"480\"")));
    QVERIFY(!html.contains(QStringLiteral("colspan")));
    QVERIFY(!html.contains(QStringLiteral(">Image<")));
    QVERIFY(!html.contains(QStringLiteral(">capture.wsresource<")));
    QVERIFY(html.contains(QStringLiteral("<img src=\"file://")));
    QVERIFY(html.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(!html.contains(QStringLiteral("width=\"338\"")));
    QVERIFY(!html.contains(QStringLiteral("height=\"352\"")));
    QVERIFY(html.contains(QStringLiteral("height:auto")));
    QVERIFY(html.contains(QStringLiteral("object-fit:contain")));
    QVERIFY(html.contains(QStringLiteral("data-source-width=\"1600\"")));
    QVERIFY(html.contains(QStringLiteral("data-source-height=\"900\"")));
    QVERIFY(html.contains(QStringLiteral("data-display-width=\"480\"")));
    QVERIFY(html.contains(QStringLiteral("data-display-height=\"270\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-display-height=\"313\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-chrome-width=\"480\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-header-height=\"24\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-toolbar-height=\"19\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-text-pixel-size=\"11\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-text-line-height=\"11\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-more-icon-size=\"16\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-more-dot-size=\"2\"")));
    QVERIFY(html.contains(QStringLiteral("data-max-width-height-ratio=\"1:1\"")));
    QVERIFY(html.contains(QStringLiteral("max-height:100%")));
    QVERIFY(!html.contains(QStringLiteral("<input")));
    QVERIFY(!html.contains(QStringLiteral("<textarea")));
    QVERIFY(!html.contains(QStringLiteral("contenteditable")));
    QVERIFY(!html.contains(QStringLiteral("font-weight:700")));
    QVERIFY(!html.contains(QStringLiteral("cellpadding=\"6\"")));

    const QRegularExpression imageSourceExpression(QStringLiteral("src=\"([^\"]+)\""));
    const QRegularExpressionMatch imageSourceMatch = imageSourceExpression.match(html);
    QVERIFY(imageSourceMatch.hasMatch());
    const QString previewPath = QUrl(imageSourceMatch.captured(1)).toLocalFile();
    QVERIFY2(!previewPath.isEmpty(), qPrintable(imageSourceMatch.captured(1)));
    const QImage previewImage(previewPath);
    QVERIFY(!previewImage.isNull());
    QCOMPARE(previewImage.size(), QSize(480, 313));

    QTextDocument richTextDocument;
    richTextDocument.setHtml(html);
    const QString richTextRoundTrip = richTextDocument.toHtml();
    QVERIFY(richTextRoundTrip.contains(QStringLiteral("<img")));
    QVERIFY(richTextRoundTrip.contains(QStringLiteral("file://")));
    QCOMPARE(richTextDocument.toPlainText(), QString(QChar::ObjectReplacementCharacter));

    const QStringList renderedTextLines =
        WhatSon::EditorComponent::ResourceFrame::renderedTextLines(descriptor);
    QVERIFY(renderedTextLines.contains(QStringLiteral("Image")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("...")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("Image...")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("Image ...")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("capture.wsresource")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("capture image .png")));
    QCOMPARE(WhatSon::EditorComponent::ResourceFrame::previewViewportSize(), QSize());
    QCOMPARE(
        WhatSon::EditorComponent::ResourceFrame::imageDisplaySize(QSize(1600, 900)),
        QSize(1600, 900));
    QCOMPARE(
        WhatSon::EditorComponent::ResourceFrame::imageDisplaySize(QSize(900, 1600)),
        QSize(900, 900));
}
