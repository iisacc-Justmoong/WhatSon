#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.hpp"

void WhatSonCppRegressionTests::editorRendererPipeline_routesIiXmlTreeThroughIiHtmlBlockObjects()
{
    const QString parserSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/parser/ContentsWsnBodyBlockParser.cpp"));
    const QString pipelineSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.cpp"));

    QVERIFY(!parserSource.isEmpty());
    QVERIFY(parserSource.contains(QStringLiteral("#include <iiXml.h>")));
    QVERIFY(parserSource.contains(QStringLiteral("iiXml::Parser::TagParser")));
    QVERIFY(parserSource.contains(QStringLiteral("ParseAllDocumentResult")));
    QVERIFY(parserSource.contains(QStringLiteral("collectExplicitSpansFromIiXmlDocument")));
    QVERIFY(!parserSource.contains(QStringLiteral("collectExplicitSpansFromLegacyTagScan")));
    QVERIFY(!parserSource.contains(QStringLiteral("kAnyTagPattern")));
    QVERIFY(!parserSource.contains(QStringLiteral("OpenExplicitBlock")));

    QVERIFY(!pipelineSource.isEmpty());
    QVERIFY(pipelineSource.contains(QStringLiteral("#include <iiXml.h>")));
    QVERIFY(pipelineSource.contains(QStringLiteral("#include <iiHtmlBlock.h>")));
    QVERIFY(pipelineSource.contains(QStringLiteral("iiHtmlBlock::iiXmlToHTML")));
    QVERIFY(pipelineSource.contains(QStringLiteral("iiHtmlBlock::DivideBlock")));
    QVERIFY(pipelineSource.contains(QStringLiteral("buildIiHtmlBlockProjection(const QVariantMap& token)")));
    QVERIFY(!pipelineSource.contains(QStringLiteral("htmlBlockProjection.blocks.at(static_cast<std::size_t>(index))")));
    QVERIFY(pipelineSource.contains(QStringLiteral("htmlBlockObjectSource")));
    QVERIFY(pipelineSource.contains(QStringLiteral("htmlBlockCount")));

    const ContentsWsnBodyBlockParser parser;
    const ContentsWsnBodyBlockParser::ParseResult malformedResult = parser.parse(
        QStringLiteral("<callout>Open callout\n<paragraph>Sibling block</paragraph>"));
    QVERIFY(malformedResult.renderedCallouts.isEmpty());

    const ContentsHtmlBlockRenderPipeline pipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult result = pipeline.renderEditorDocument(
        QStringLiteral(
            "<paragraph>Alpha</paragraph>\n"
            "<resource type=\"image\" path=\"cover.png\" />\n"
            "<callout>Beta</callout>\n"
            "</break>"));

    QVERIFY(result.normalizedHtmlBlocks.size() >= result.htmlTokens.size());
    QVERIFY(result.normalizedHtmlBlocks.size() >= 4);
    for (const QVariant& blockValue : result.normalizedHtmlBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        QCOMPARE(block.value(QStringLiteral("htmlBlockObjectSource")).toString(), QStringLiteral("iiHtmlBlock"));
        QVERIFY(!block.value(QStringLiteral("htmlBlockTagName")).toString().isEmpty());
        QVERIFY(block.value(QStringLiteral("htmlBlockIsDisplayBlock")).toBool());
        QVERIFY(block.value(QStringLiteral("htmlBlockIndex")).toInt() >= 0);
        QVERIFY(block.value(QStringLiteral("htmlTokenStartIndex")).toInt() >= 0);
    }
}

void WhatSonCppRegressionTests::editorRendererPipeline_materializesEnterNewlinesAsParagraphSlots()
{
    const ContentsHtmlBlockRenderPipeline pipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult result = pipeline.renderEditorDocument(
        QStringLiteral(
            "<paragraph>Alpha\n"
            "\n"
            "Beta</paragraph>\n"
            "<paragraph>Gamma</paragraph>"));

    QVERIFY(!result.documentHtml.isEmpty());
    QVERIFY(!result.documentHtml.contains(QStringLiteral("Alpha<br/>")));
    QVERIFY(result.documentHtml.contains(QStringLiteral(">Alpha</p>")));
    QVERIFY(result.documentHtml.contains(QStringLiteral("<p style=\"margin-top:0px;margin-bottom:0px;\">&nbsp;</p>")));
    QVERIFY(result.documentHtml.contains(QStringLiteral(">Beta</p>")));
    QVERIFY(result.documentHtml.contains(QStringLiteral(">Gamma</p>")));

    int paragraphCount = 0;
    qsizetype cursor = 0;
    while ((cursor = result.documentHtml.indexOf(QStringLiteral("<p "), cursor)) >= 0)
    {
        ++paragraphCount;
        cursor += 3;
    }
    QCOMPARE(paragraphCount, 4);
}

void WhatSonCppRegressionTests::editorRendererPipeline_rendersInlineStyleTagsAsRichTextHtml()
{
    const ContentsHtmlBlockRenderPipeline pipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult result = pipeline.renderEditorDocument(
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"));

    QVERIFY(!result.documentHtml.isEmpty());
    QVERIFY(!result.documentHtml.contains(QStringLiteral("&lt;bold&gt;")));
    QVERIFY(!result.documentHtml.contains(QStringLiteral("&lt;italic&gt;")));
    QVERIFY(!result.documentHtml.contains(QStringLiteral("<bold>")));
    QVERIFY(!result.documentHtml.contains(QStringLiteral("<italic>")));
    QVERIFY(result.documentHtml.contains(QStringLiteral("<strong style=\"font-weight:900;\">Al")));
    QVERIFY(result.documentHtml.contains(QStringLiteral("<span style=\"font-style:italic;\">pha</span>")));
    QVERIFY(result.documentHtml.contains(QStringLiteral("<span style=\"font-style:italic;\">&nbsp;Beta</span>")));

    QVERIFY(!result.htmlTokens.isEmpty());
    const QVariantMap token = result.htmlTokens.first().toMap();
    const QString tokenHtml = token.value(QStringLiteral("html")).toString();
    QVERIFY(token.value(QStringLiteral("overlayVisible")).toBool());
    QVERIFY(tokenHtml.contains(QStringLiteral("<strong style=\"font-weight:900;\">Al")));
    QVERIFY(tokenHtml.contains(QStringLiteral("<span style=\"font-style:italic;\">pha</span>")));
    QVERIFY(!tokenHtml.contains(QStringLiteral("&lt;bold&gt;")));

    bool foundStyledHtmlBlock = false;
    for (const QVariant& blockValue : result.normalizedHtmlBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        const QString blockHtml = block.value(QStringLiteral("htmlBlockHtml")).toString();
        if (blockHtml.contains(QStringLiteral("font-weight:900"))
            && blockHtml.contains(QStringLiteral("font-style:italic")))
        {
            foundStyledHtmlBlock = true;
        }
        QVERIFY(!blockHtml.contains(QStringLiteral("&lt;bold&gt;")));
        QVERIFY(!blockHtml.contains(QStringLiteral("&lt;italic&gt;")));
    }
    QVERIFY(foundStyledHtmlBlock);
}
