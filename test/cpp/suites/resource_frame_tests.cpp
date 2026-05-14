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

    QVERIFY(html.startsWith(QStringLiteral("<!--whatson-resource-source:")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-resource-frame\"")));
    QVERIFY(html.contains(QStringLiteral("data-figma-node-id=\"292:50\"")));
    QVERIFY(html.contains(QStringLiteral("resourceHeader")));
    QVERIFY(html.contains(QStringLiteral("resourceToolbar")));
    QVERIFY(html.contains(QStringLiteral("whatson-resource-more")));
    QVERIFY(html.contains(QStringLiteral("width=\"480\"")));
    QVERIFY(!html.contains(QStringLiteral("colspan")));
    QVERIFY(html.contains(QStringLiteral("border-color:#2C2E2F")));
    QVERIFY(html.contains(QStringLiteral("border-radius:12px")));
    QVERIFY(html.contains(QStringLiteral(">Image<")));
    QVERIFY(html.contains(QStringLiteral(">capture.wsresource<")));
    QVERIFY(html.contains(QStringLiteral("<img src=\"file://")));
    QVERIFY(html.contains(QStringLiteral("width=\"338\"")));
    QVERIFY(html.contains(QStringLiteral("height=\"190\"")));
    QVERIFY(!html.contains(QStringLiteral("font-weight:700")));
    QVERIFY(!html.contains(QStringLiteral("cellpadding=\"6\"")));

    QTextDocument richTextDocument;
    richTextDocument.setHtml(html);
    const QString richTextRoundTrip = richTextDocument.toHtml();
    QVERIFY(richTextRoundTrip.contains(QStringLiteral("<img")));
    QVERIFY(richTextRoundTrip.contains(QStringLiteral("file://")));

    const QStringList renderedTextLines =
        WhatSon::EditorComponent::ResourceFrame::renderedTextLines(descriptor);
    QVERIFY(renderedTextLines.contains(QStringLiteral("Image")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("...")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("Image ...")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("capture.wsresource")));
    QVERIFY(renderedTextLines.contains(QStringLiteral("capture image .png")));
    QCOMPARE(WhatSon::EditorComponent::ResourceFrame::previewViewportSize(), QSize(338, 352));
}
