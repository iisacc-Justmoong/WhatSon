#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.hpp"
#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"
#include "app/models/editor/tags/ContentsResourceTagController.hpp"
#include "app/models/file/viewer/ContentsBodyResourceRenderer.hpp"

void WhatSonCppRegressionTests::resourceRenderer_resolvesIiXmlResourceTagsAndStructuredVisualBlocks()
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
    QCOMPARE(frameImage.size(), QSize(320, 260));
    QCOMPARE(frameImage.pixelColor(0, 0).alpha(), 0);
    QVERIFY(frameImage.pixelColor(160, 130).alpha() > 0);

    const ContentsHtmlBlockRenderPipeline pipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult renderResult = pipeline.renderEditorDocument(sourceText);
    QVERIFY(!renderResult.documentHtml.contains(sourceText));
    QVERIFY(!renderResult.documentHtml.contains(QStringLiteral("whatson-resource-block")));
    QVERIFY(renderResult.documentHtml.contains(QStringLiteral("&nbsp;")));

    const QString legacyPlaceholderHtml =
        QStringLiteral("<!--whatson-resource-block:0--><p style=\"margin-top:0px;margin-bottom:0px;\">&nbsp;</p><!--/whatson-resource-block:0-->");
    const QString inlineHtml =
        inlinePresentation.renderEditorSurfaceHtmlWithInlineResources(legacyPlaceholderHtml);
    QVERIFY(!inlineHtml.contains(QStringLiteral("<resource")));
    QVERIFY(!inlineHtml.contains(QStringLiteral("whatson-resource-block")));
    QVERIFY(inlineHtml.contains(QStringLiteral("<img")));
    QVERIFY(inlineHtml.contains(QStringLiteral("width=\"320\" height=\"260\"")));
    QVERIFY(inlineHtml.contains(QStringLiteral("line-height:0px")));
    QVERIFY(inlineHtml.contains(QStringLiteral("style=\"vertical-align:top;\"")));
    QVERIFY(inlineHtml.contains(frameImageSource));
    QVERIFY(inlineHtml.contains(QUrl::fromLocalFile(assetPath).toString()));
    const QVariantMap visualBlock = inlinePresentation.inlineResourceVisualBlocks().constFirst().toMap();
    QCOMPARE(visualBlock.value(QStringLiteral("width")).toInt(), 320);
    QCOMPARE(visualBlock.value(QStringLiteral("height")).toInt(), 260);
    QCOMPARE(visualBlock.value(QStringLiteral("visualHeight")).toInt(), 260);
    QCOMPARE(visualBlock.value(QStringLiteral("imageSource")).toString(), frameImageSource);
    QVERIFY(visualBlock.value(QStringLiteral("renderable")).toBool());
    const QVariantList htmlTokens{
        QVariantMap{
            {QStringLiteral("html"), QStringLiteral("<p style=\"margin-top:0px;margin-bottom:0px;\">&nbsp;</p>")},
            {QStringLiteral("ownsBlockFlow"), true},
            {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
        },
        QVariantMap{
            {QStringLiteral("html"), QStringLiteral("<p style=\"margin-top:0px;margin-bottom:0px;\">After</p>")},
            {QStringLiteral("ownsBlockFlow"), true},
            {QStringLiteral("renderDelegateType"), QStringLiteral("text")},
        },
    };
    const QString flowHtml = inlinePresentation.editorSurfaceHtmlWithResourceVisualBlocks(
        htmlTokens,
        QVariantList{visualBlock},
        renderResult.documentHtml);
    QVERIFY(!flowHtml.contains(QStringLiteral("<img")));
    QVERIFY(flowHtml.contains(QStringLiteral("line-height:262px")));
    QVERIFY(flowHtml.contains(QStringLiteral("font-size:1px;color:transparent")));
    QVERIFY(flowHtml.contains(QStringLiteral(">After</p>")));
    const QString tokenOwnedBlockHtml = inlinePresentation.editorSurfaceHtmlWithResourceVisualBlocks(
        QVariantList{
            QVariantMap{
                {QStringLiteral("html"), QStringLiteral("<div>Token owned</div>")},
                {QStringLiteral("ownsBlockFlow"), true},
                {QStringLiteral("renderDelegateType"), QStringLiteral("text")},
            },
        },
        QVariantList{},
        QString());
    QCOMPARE(tokenOwnedBlockHtml, QStringLiteral("<div>Token owned</div>"));
    const QString tokenInlineHtml = inlinePresentation.editorSurfaceHtmlWithResourceVisualBlocks(
        QVariantList{
            QVariantMap{
                {QStringLiteral("html"), QStringLiteral("<div>Token inline</div>")},
                {QStringLiteral("ownsBlockFlow"), false},
                {QStringLiteral("renderDelegateType"), QStringLiteral("text")},
            },
        },
        QVariantList{},
        QString());
    QVERIFY(tokenInlineHtml.startsWith(QStringLiteral("<p")));
    QVERIFY(tokenInlineHtml.contains(QStringLiteral("<div>Token inline</div>")));

    const QString fullWidthInlineHtml =
        inlinePresentation.renderEditorSurfaceHtmlWithInlineResources(
            legacyPlaceholderHtml,
            bodyRenderer.renderedResources(),
            640);
    QVERIFY(fullWidthInlineHtml.contains(QStringLiteral("width=\"640\" height=\"520\"")));
    QCOMPARE(
        inlinePresentation.inlineResourceVisualBlocks(bodyRenderer.renderedResources(), 640)
            .constFirst()
            .toMap()
            .value(QStringLiteral("height"))
            .toInt(),
        520);
}
