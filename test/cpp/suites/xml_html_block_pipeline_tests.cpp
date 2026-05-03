#include "test/cpp/whatson_cpp_regression_tests.hpp"

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

    QVERIFY(!pipelineSource.isEmpty());
    QVERIFY(pipelineSource.contains(QStringLiteral("#include <iiXml.h>")));
    QVERIFY(pipelineSource.contains(QStringLiteral("#include <iiHtmlBlock.h>")));
    QVERIFY(pipelineSource.contains(QStringLiteral("iiHtmlBlock::iiXmlToHTML")));
    QVERIFY(pipelineSource.contains(QStringLiteral("iiHtmlBlock::DivideBlock")));
    QVERIFY(pipelineSource.contains(QStringLiteral("htmlBlockObjectSource")));

    const ContentsHtmlBlockRenderPipeline pipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult result = pipeline.renderEditorDocument(
        QStringLiteral(
            "<paragraph>Alpha</paragraph>\n"
            "<resource type=\"image\" path=\"cover.png\" />\n"
            "<callout>Beta</callout>\n"
            "</break>"));

    QVERIFY(result.normalizedHtmlBlocks.size() >= 4);
    for (const QVariant& blockValue : result.normalizedHtmlBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        QCOMPARE(block.value(QStringLiteral("htmlBlockObjectSource")).toString(), QStringLiteral("iiHtmlBlock"));
        QVERIFY(!block.value(QStringLiteral("htmlBlockTagName")).toString().isEmpty());
        QVERIFY(block.value(QStringLiteral("htmlBlockIsDisplayBlock")).toBool());
    }
}
