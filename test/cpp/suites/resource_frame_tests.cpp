#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

void WhatSonCppRegressionTests::resourceFrame_rendersImageOnlyContainer()
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
    descriptor.editorViewportWidth = 960;

    const QString html = WhatSon::EditorComponent::ResourceFrame::renderHtml(descriptor);

    QVERIFY(html.contains(QStringLiteral("<!--whatson-resource-source:")));
    QVERIFY(html.startsWith(QStringLiteral("<!--whatson-resource-source:")));
    QVERIFY(html.contains(QStringLiteral("class=\"whatson-resource-frame whatson-resource-media\"")));
    QVERIFY(!html.contains(QStringLiteral("<table")));
    QVERIFY(!html.contains(QStringLiteral("</table>")));
    QVERIFY(html.contains(QStringLiteral("margin-top:0px")));
    QVERIFY(html.contains(QStringLiteral("margin-bottom:0px")));
    QVERIFY(html.contains(QStringLiteral("margin-left:0px")));
    QVERIFY(html.contains(QStringLiteral("margin-right:0px")));
    QVERIFY(!html.contains(QStringLiteral("data-whatson-component=\"resource-frame\"")));
    QVERIFY(html.count(QStringLiteral("<img")) == 1);
    QVERIFY(html.contains(QStringLiteral("width:100%")));
    QVERIFY(html.contains(QStringLiteral("max-width:100%")));
    QVERIFY(html.contains(QStringLiteral("data-figma-node-id=\"292:50\"")));
    QVERIFY(html.contains(QStringLiteral("data-resource-preview=\"image-only-frame\"")));
    QVERIFY(!html.contains(QStringLiteral("data-resource-preview=\"structured-frame\"")));
    QVERIFY(!html.contains(QStringLiteral("data-resource-preview=\"single-object-raster\"")));
    QVERIFY(!html.contains(QStringLiteral("data-resource-type-label")));
    QVERIFY(!html.contains(QStringLiteral("data-resource-file-name")));
    QVERIFY(!html.contains(QStringLiteral("resourceHeader")));
    QVERIFY(!html.contains(QStringLiteral("resourceToolbar")));
    QVERIFY(!html.contains(QStringLiteral("whatson-resource-more")));
    QVERIFY(!html.contains(QStringLiteral("whatson-resource-type-display")));
    QVERIFY(!html.contains(QStringLiteral("whatson-resource-filename-display")));
    QVERIFY(!html.contains(QStringLiteral("data-display-role=\"resource-type\"")));
    QVERIFY(!html.contains(QStringLiteral("data-display-role=\"resource-file-name\"")));
    QVERIFY(!html.contains(QStringLiteral(" width=\"480\"")));
    QVERIFY(!html.contains(QStringLiteral("colspan=\"2\"")));
    QVERIFY(!html.contains(QStringLiteral(">Image<")));
    QVERIFY(!html.contains(QStringLiteral(">...<")));
    QVERIFY(!html.contains(QStringLiteral(">capture.wsresource<")));
    QVERIFY(!html.contains(QStringLiteral("capture.wsresource")));
    QVERIFY(html.contains(QStringLiteral("<img src=\"file://")));
    QVERIFY(html.contains(QStringLiteral("alt=\"\"")));
    QVERIFY(html.contains(QStringLiteral("width=\"960\"")));
    QVERIFY(html.contains(QStringLiteral("height=\"540\"")));
    QVERIFY(!html.contains(QStringLiteral("width=\"338\"")));
    QVERIFY(!html.contains(QStringLiteral("height=\"352\"")));
    QVERIFY(html.contains(QStringLiteral("vertical-align:top")));
    QVERIFY(html.contains(QStringLiteral("object-fit:contain")));
    QVERIFY(html.contains(QStringLiteral("data-source-width=\"1600\"")));
    QVERIFY(html.contains(QStringLiteral("data-source-height=\"900\"")));
    QVERIFY(html.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(html.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(html.contains(QStringLiteral("data-display-left=\"0\"")));
    QVERIFY(html.contains(QStringLiteral("data-display-top=\"0\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-design-width=\"480\"")));
    QVERIFY(html.contains(QStringLiteral("data-frame-render-width=\"960\"")));
    QVERIFY(html.contains(QStringLiteral("data-media-alignment=\"center\"")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-header-height")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-toolbar-height")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-text-pixel-size")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-text-line-height")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-more-icon-size")));
    QVERIFY(!html.contains(QStringLiteral("data-frame-more-dot-size")));
    QVERIFY(html.contains(QStringLiteral("data-max-width-height-ratio=\"1:1\"")));
    QVERIFY(html.contains(QStringLiteral("max-height:100%")));
    QVERIFY(!html.contains(QStringLiteral("<input")));
    QVERIFY(!html.contains(QStringLiteral("<textarea")));
    QVERIFY(!html.contains(QStringLiteral("contenteditable")));
    QVERIFY(!html.contains(QStringLiteral("font-weight:700")));
    QVERIFY(!html.contains(QStringLiteral("cellpadding=\"6\"")));

    const auto previewPathFromHtml = [](const QString& renderedHtml) {
        const QRegularExpression imageSourceExpression(QStringLiteral("src=\"([^\"]+)\""));
        const QRegularExpressionMatch imageSourceMatch = imageSourceExpression.match(renderedHtml);
        return imageSourceMatch.hasMatch()
            ? QUrl(imageSourceMatch.captured(1)).toLocalFile()
            : QString();
    };

    const QString previewPath = previewPathFromHtml(html);
    QVERIFY2(!previewPath.isEmpty(), qPrintable(html));
    const QImage previewImage(previewPath);
    QVERIFY(!previewImage.isNull());
    QCOMPARE(previewImage.size(), QSize(960, 540));
    QCOMPARE(previewImage.pixelColor(0, 0), sourceImage.pixelColor(0, 0));
    QCOMPARE(previewImage.pixelColor(previewImage.width() - 1, previewImage.height() - 1), sourceImage.pixelColor(0, 0));

    const QString centeredImagePath = QDir(imageDirectory.path()).filePath(QStringLiteral("centered-capture.png"));
    QImage centeredSourceImage(QSize(320, 180), QImage::Format_ARGB32_Premultiplied);
    centeredSourceImage.fill(qRgba(220, 45, 90, 255));
    QVERIFY(centeredSourceImage.save(centeredImagePath));

    descriptor.sourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/centered.wsresource\" id=\"centered\" />");
    descriptor.resourcePath = QStringLiteral("Workspace.wsresources/centered.wsresource");
    descriptor.resourceId = QStringLiteral("centered");
    descriptor.resolvedAssetPath = centeredImagePath;
    descriptor.editorViewportWidth = 960;

    const QString centeredHtml = WhatSon::EditorComponent::ResourceFrame::renderHtml(descriptor);
    QVERIFY2(centeredHtml.contains(QStringLiteral("data-display-width=\"320\"")), qPrintable(centeredHtml));
    QVERIFY(centeredHtml.contains(QStringLiteral("data-display-height=\"180\"")));
    QVERIFY(centeredHtml.contains(QStringLiteral("data-display-left=\"320\"")));
    QVERIFY(centeredHtml.contains(QStringLiteral("data-display-top=\"0\"")));
    QVERIFY(centeredHtml.contains(QStringLiteral("data-frame-render-width=\"960\"")));
    QVERIFY(centeredHtml.contains(QStringLiteral("data-frame-display-height=\"180\"")));
    QVERIFY(centeredHtml.contains(QStringLiteral("width=\"960\"")));
    QVERIFY(centeredHtml.contains(QStringLiteral("height=\"180\"")));

    const QImage centeredPreviewImage(previewPathFromHtml(centeredHtml));
    QVERIFY(!centeredPreviewImage.isNull());
    QCOMPARE(centeredPreviewImage.size(), QSize(960, 180));
    QCOMPARE(centeredPreviewImage.pixelColor(319, 90), QColor(QStringLiteral("#1E1F20")));
    QCOMPARE(centeredPreviewImage.pixelColor(320, 90), centeredSourceImage.pixelColor(0, 0));
    QCOMPARE(centeredPreviewImage.pixelColor(639, 90), centeredSourceImage.pixelColor(0, 0));
    QCOMPARE(centeredPreviewImage.pixelColor(640, 90), QColor(QStringLiteral("#1E1F20")));

    descriptor.editorViewportWidth = 1200;

    const QString widerCenteredHtml = WhatSon::EditorComponent::ResourceFrame::renderHtml(descriptor);
    QVERIFY(widerCenteredHtml.contains(QStringLiteral("data-display-width=\"320\"")));
    QVERIFY(widerCenteredHtml.contains(QStringLiteral("data-display-height=\"180\"")));
    QVERIFY(widerCenteredHtml.contains(QStringLiteral("data-display-left=\"440\"")));
    QVERIFY(widerCenteredHtml.contains(QStringLiteral("data-frame-render-width=\"1200\"")));

    const QImage widerCenteredPreviewImage(previewPathFromHtml(widerCenteredHtml));
    QVERIFY(!widerCenteredPreviewImage.isNull());
    QCOMPARE(widerCenteredPreviewImage.size(), QSize(1200, 180));
    QCOMPARE(widerCenteredPreviewImage.pixelColor(439, 90), QColor(QStringLiteral("#1E1F20")));
    QCOMPARE(widerCenteredPreviewImage.pixelColor(440, 90), centeredSourceImage.pixelColor(0, 0));
    QCOMPARE(widerCenteredPreviewImage.pixelColor(759, 90), centeredSourceImage.pixelColor(0, 0));
    QCOMPARE(widerCenteredPreviewImage.pixelColor(760, 90), QColor(QStringLiteral("#1E1F20")));

    descriptor.sourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    descriptor.resourcePath = QStringLiteral("Workspace.wsresources/capture.wsresource");
    descriptor.resourceId = QStringLiteral("capture");
    descriptor.resolvedAssetPath = imagePath;
    descriptor.editorViewportWidth = 1200;
    descriptor.lockedFrameDisplayHeight = 540;

    const QString widerLockedHeightHtml = WhatSon::EditorComponent::ResourceFrame::renderHtml(descriptor);
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("data-display-left=\"120\"")));
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("data-frame-render-width=\"1200\"")));
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("width=\"1200\"")));
    QVERIFY(widerLockedHeightHtml.contains(QStringLiteral("height=\"540\"")));

    const QImage widerLockedHeightPreviewImage(previewPathFromHtml(widerLockedHeightHtml));
    QVERIFY(!widerLockedHeightPreviewImage.isNull());
    QCOMPARE(widerLockedHeightPreviewImage.size(), QSize(1200, 540));
    QCOMPARE(widerLockedHeightPreviewImage.pixelColor(119, 270), QColor(QStringLiteral("#1E1F20")));
    QCOMPARE(widerLockedHeightPreviewImage.pixelColor(120, 270), sourceImage.pixelColor(0, 0));
    QCOMPARE(widerLockedHeightPreviewImage.pixelColor(1079, 270), sourceImage.pixelColor(0, 0));
    QCOMPARE(widerLockedHeightPreviewImage.pixelColor(1080, 270), QColor(QStringLiteral("#1E1F20")));

    descriptor.editorViewportWidth = 720;

    const QString narrowerLockedHeightHtml = WhatSon::EditorComponent::ResourceFrame::renderHtml(descriptor);
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("data-display-width=\"960\"")));
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("data-display-height=\"540\"")));
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("data-display-left=\"-120\"")));
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("data-frame-render-width=\"720\"")));
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("data-frame-display-height=\"540\"")));
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("width=\"720\"")));
    QVERIFY(narrowerLockedHeightHtml.contains(QStringLiteral("height=\"540\"")));

    const QImage narrowerLockedHeightPreviewImage(previewPathFromHtml(narrowerLockedHeightHtml));
    QVERIFY(!narrowerLockedHeightPreviewImage.isNull());
    QCOMPARE(narrowerLockedHeightPreviewImage.size(), QSize(720, 540));
    QCOMPARE(narrowerLockedHeightPreviewImage.pixelColor(0, 270), sourceImage.pixelColor(0, 0));
    QCOMPARE(narrowerLockedHeightPreviewImage.pixelColor(719, 270), sourceImage.pixelColor(0, 0));

    QTextDocument richTextDocument;
    richTextDocument.setHtml(html);
    const QString richTextRoundTrip = richTextDocument.toHtml();
    const QString richTextPlainText = richTextDocument.toPlainText();
    QVERIFY(richTextRoundTrip.contains(QStringLiteral("<img")));
    QVERIFY(richTextRoundTrip.contains(QStringLiteral("file://")));
    QVERIFY(!richTextPlainText.contains(QStringLiteral("Image")));
    QVERIFY(!richTextPlainText.contains(QStringLiteral("capture.wsresource")));
    QVERIFY(richTextPlainText.contains(QChar::ObjectReplacementCharacter));

    QCOMPARE(WhatSon::EditorComponent::ResourceFrame::previewViewportSize(), QSize());
    QCOMPARE(
        WhatSon::EditorComponent::ResourceFrame::imageDisplaySize(QSize(1600, 900)),
        QSize(1600, 900));
    QCOMPARE(
        WhatSon::EditorComponent::ResourceFrame::imageDisplaySize(QSize(900, 1600)),
        QSize(900, 900));
}
