#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaLayer.qml"));
    const QString calloutLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutLayer.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaLayerSource.isEmpty());
    QVERIFY(!calloutLayerSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("paperPaletteEnabled: documentFlow.paperPaletteEnabled")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("paperPaletteEnabled: documentBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(
        QStringLiteral("readonly property color editorTextColor: paperPaletteEnabled ? \"#111111\" : LV.Theme.bodyColor")));
    QVERIFY(textBlockSource.contains(QStringLiteral("paperPaletteEnabled: textBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(QStringLiteral("textColor: textBlock.editorTextColor")));

    QVERIFY(!agendaBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("color: \"#80FFFFFF\"")));
    QVERIFY(agendaBlockSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutBlockSource.contains(
        QStringLiteral("readonly property color bodyTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaLayerSource.contains(QStringLiteral("color: \"#FFFFFF\"")));
    QVERIFY(agendaLayerSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutLayerSource.contains(
        QStringLiteral("readonly property color textColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("paperPaletteEnabled: contentsView.showPrintEditorLayout")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_clipInlineResourceCardsToMeasuredBlockBounds()
{
    const QString resourceBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceBlock.qml"));
    const QString resourceCardSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceRenderCard.qml"));

    QVERIFY(!resourceBlockSource.isEmpty());
    QVERIFY(!resourceCardSource.isEmpty());

    QVERIFY(resourceBlockSource.contains(QStringLiteral("implicitHeight: resourceCard.implicitHeight")));
    QVERIFY(resourceBlockSource.contains(QStringLiteral("clip: true")));
    QVERIFY(resourceCardSource.contains(QStringLiteral("clip: true")));
    QVERIFY(!resourceCardSource.contains(QStringLiteral("clip: false")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_consumeRendererNormalizedBlocksWithoutLocalFlattening()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("documentHost.documentBlocks = documentHost.collectionPolicy.normalizeEntries(documentFlow.documentBlocks)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function flattenedInteractiveBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function normalizedParsedBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function buildFlattenedInteractiveTextGroup(")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function implicitTextBlockInteractiveFlattenCandidate(")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("function snapshotTokenForLogicalLine(blockEntry, logicalLines, lineIndex)")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("\"snapshotToken\": documentFlow.snapshotTokenForLogicalLine(blockEntry, logicalLines, index)")));
}
