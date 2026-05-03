#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.hpp"
#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"
#include "app/models/editor/tags/ContentsResourceTagController.hpp"
#include "app/models/file/viewer/ContentsBodyResourceRenderer.hpp"

void WhatSonCppRegressionTests::resourceRenderer_resolvesIiXmlResourceTagsAndInlineHtmlBlockPlaceholders()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString resourceId = QStringLiteral("2btafa5lb6sj8fq2hlac4lnbhxttytja");
    const QString resourceReference = QStringLiteral("icloud.wsresources/%1.wsresource").arg(resourceId);
    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("Demo.wshub"));
    const QString noteDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents/note.wsnote"));
    const QString packageDirectoryPath = QDir(hubPath).filePath(resourceReference);
    QVERIFY(QDir().mkpath(noteDirectoryPath));
    QVERIFY(QDir().mkpath(packageDirectoryPath));

    const QString assetPath = QDir(packageDirectoryPath).filePath(QStringLiteral("image.png"));
    QImage sourceImage(QSize(11, 7), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(24, 48, 96, 255));
    QVERIFY(sourceImage.save(assetPath));

    const WhatSon::Resources::ResourcePackageMetadata metadata =
        WhatSon::Resources::buildMetadataForAssetFile(assetPath, resourceId, resourceReference);
    QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(metadataFile.write(WhatSon::Resources::createResourcePackageMetadataXml(metadata).toUtf8()) >= 0);
    metadataFile.close();

    const QString sourceText = QStringLiteral(
        "<resource type=\"image\" format=\".png\" path=\"icloud.wsresources/"
        "2btafa5lb6sj8fq2hlac4lnbhxttytja.wsresource\" id=\"2btafa5lb6sj8fq2hlac4lnbhxttytja\" />");

    const ContentsWsnBodyBlockParser parser;
    const ContentsWsnBodyBlockParser::ParseResult parseResult = parser.parse(sourceText);
    QCOMPARE(parseResult.renderedDocumentBlocks.size(), 1);

    const QVariantMap resourceBlock = parseResult.renderedDocumentBlocks.constFirst().toMap();
    QCOMPARE(resourceBlock.value(QStringLiteral("type")).toString(), QStringLiteral("resource"));
    QCOMPARE(resourceBlock.value(QStringLiteral("resourceType")).toString(), QStringLiteral("image"));
    QCOMPARE(resourceBlock.value(QStringLiteral("resourceFormat")).toString(), QStringLiteral(".png"));
    QCOMPARE(resourceBlock.value(QStringLiteral("resourcePath")).toString(), resourceReference);
    QCOMPARE(resourceBlock.value(QStringLiteral("resourceId")).toString(), resourceId);

    ContentsBodyResourceRenderer bodyRenderer;
    bodyRenderer.setNoteDirectoryPath(noteDirectoryPath);
    bodyRenderer.setMaxRenderCount(0);
    bodyRenderer.setDocumentBlocks(parseResult.renderedDocumentBlocks);
    QCOMPARE(bodyRenderer.resourceCount(), 1);

    const QVariantMap renderedResource = bodyRenderer.renderedResources().constFirst().toMap();
    QCOMPARE(renderedResource.value(QStringLiteral("renderMode")).toString(), QStringLiteral("image"));
    QCOMPARE(renderedResource.value(QStringLiteral("resourceId")).toString(), resourceId);
    QCOMPARE(renderedResource.value(QStringLiteral("resourcePath")).toString(), resourceReference);
    QCOMPARE(QDir::cleanPath(renderedResource.value(QStringLiteral("resolvedPath")).toString()), QDir::cleanPath(assetPath));
    QCOMPARE(renderedResource.value(QStringLiteral("source")).toString(), QUrl::fromLocalFile(assetPath).toString());
    QCOMPARE(renderedResource.value(QStringLiteral("imageWidth")).toInt(), 11);
    QCOMPARE(renderedResource.value(QStringLiteral("imageHeight")).toInt(), 7);

    ContentsResourceTagController resourceTagController;
    ContentsInlineResourcePresentationController inlinePresentation;
    QVERIFY(inlinePresentation.setProperty("bodyResourceRenderer", QVariant::fromValue<QObject*>(&bodyRenderer)));
    QVERIFY(inlinePresentation.setProperty("resourceTagController", QVariant::fromValue<QObject*>(&resourceTagController)));
    QVERIFY(inlinePresentation.setProperty("inlineHtmlImageRenderingEnabled", true));
    QVERIFY(inlinePresentation.setProperty("printPaperTextWidth", 320.0));
    QVERIFY(inlinePresentation.setProperty("showPrintEditorLayout", true));
    QCOMPARE(
        inlinePresentation.resourceEntryFrameLabel(renderedResource),
        QStringLiteral("image.png"));
    const QString frameImageSource = inlinePresentation.resourceEntryFrameImageSource(renderedResource);
    QVERIFY(frameImageSource.startsWith(QStringLiteral("file:")));
    const QImage frameImage(QUrl(frameImageSource).toLocalFile());
    QCOMPARE(frameImage.size(), QSize(480, 390));
    QCOMPARE(frameImage.pixelColor(0, 0).alpha(), 0);
    QVERIFY(frameImage.pixelColor(240, 195).alpha() > 0);

    const ContentsHtmlBlockRenderPipeline pipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult renderResult = pipeline.renderEditorDocument(sourceText);
    QVERIFY(!renderResult.documentHtml.contains(sourceText));
    QVERIFY(renderResult.documentHtml.contains(QStringLiteral("whatson-resource-block:0")));

    const QString inlineHtml =
        inlinePresentation.renderEditorSurfaceHtmlWithInlineResources(renderResult.documentHtml);
    QVERIFY(!inlineHtml.contains(QStringLiteral("<resource")));
    QVERIFY(!inlineHtml.contains(QStringLiteral("whatson-resource-block")));
    QVERIFY(inlineHtml.contains(QStringLiteral("<img")));
    QVERIFY(inlineHtml.contains(QStringLiteral("width=\"480\" height=\"390\"")));
    QVERIFY(inlineHtml.contains(frameImageSource));
    QVERIFY(inlineHtml.contains(QUrl::fromLocalFile(assetPath).toString()));
}
