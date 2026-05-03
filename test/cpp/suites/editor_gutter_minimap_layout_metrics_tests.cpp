#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsGutterLayoutMetrics_resolvesRuntimeAndDesignMetrics()
{
    ContentsGutterLayoutMetrics metrics;
    metrics.setGapNone(0);
    metrics.setGap2(2);
    metrics.setGap3(3);
    metrics.setGap5(5);
    metrics.setGap7(7);
    metrics.setGap14(14);
    metrics.setGap20(20);
    metrics.setGap24(24);
    metrics.setStrokeThin(1);
    metrics.setControlHeightMd(32);
    metrics.setControlHeightSm(24);
    metrics.setDialogMaxWidth(640);
    metrics.setInputWidthMd(320);

    QCOMPARE(metrics.changedMarkerHeight(), 40);
    QCOMPARE(metrics.changedMarkerY(), 357);
    QCOMPARE(metrics.conflictMarkerHeight(), 120);
    QCOMPARE(metrics.conflictMarkerY(), 663);
    QCOMPARE(metrics.defaultGutterWidth(), 74);
    QCOMPARE(metrics.defaultActiveLineNumber(), 27);
    QCOMPARE(metrics.designLineNumberCount(), 39);
    QCOMPARE(metrics.effectiveGutterWidth(), 74);
    QCOMPARE(metrics.iconRailX(), 40);
    QCOMPARE(metrics.lineNumberColumnLeft(), 14);
    QCOMPARE(metrics.lineNumberColumnTextWidth(), 24);

    metrics.setLogicalLineCount(0);
    QCOMPARE(metrics.effectiveLineNumberCount(), 1);
    QCOMPARE(metrics.inactiveLineNumber(), -1);
    QCOMPARE(metrics.lineNumberBaseOffset(), 1);

    metrics.setLogicalLineCount(128);
    metrics.setGutterWidthOverride(96);
    metrics.setLineNumberColumnLeftOverride(18);
    metrics.setLineNumberColumnTextWidthOverride(31);
    QCOMPARE(metrics.effectiveLineNumberCount(), 128);
    QCOMPARE(metrics.effectiveGutterWidth(), 96);
    QCOMPARE(metrics.lineNumberColumnLeft(), 18);
    QCOMPARE(metrics.lineNumberColumnTextWidth(), 31);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_projectsFallbackLineYEntries()
{
    ContentsGutterLineNumberGeometry geometry;
    geometry.setSourceText(QStringLiteral("first\nsecond\nthird"));
    geometry.setLineNumberCount(3);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(16.0);
    geometry.setFallbackLineHeight(22.0);

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);

    const QVariantMap firstEntry = entries.at(0).toMap();
    const QVariantMap secondEntry = entries.at(1).toMap();
    const QVariantMap thirdEntry = entries.at(2).toMap();
    QCOMPARE(firstEntry.value(QStringLiteral("lineNumber")).toInt(), 1);
    QCOMPARE(firstEntry.value(QStringLiteral("y")).toReal(), 16.0);
    QCOMPARE(secondEntry.value(QStringLiteral("lineNumber")).toInt(), 2);
    QCOMPARE(secondEntry.value(QStringLiteral("y")).toReal(), 38.0);
    QCOMPARE(thirdEntry.value(QStringLiteral("lineNumber")).toInt(), 3);
    QCOMPARE(thirdEntry.value(QStringLiteral("y")).toReal(), 60.0);

    geometry.setSourceText(QString());
    geometry.setLineNumberCount(0);
    QCOMPARE(geometry.lineNumberEntries().size(), 1);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_samplesEditorGeometryForRawSourceLines()
{
    ContentsGutterLineNumberGeometry geometry;
    FakeGutterDisplayGeometryHost geometryHost;

    geometry.setEditorGeometryHost(&geometryHost);
    geometry.setSourceText(QStringLiteral("first\nsecond\nthird"));
    geometry.setLogicalLineStartOffsets(QVariantList{0, 12, 24});
    geometry.setLineNumberCount(3);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(80.0);
    geometry.setFallbackLineHeight(20.0);
    geometryHost.clearSampledPositions();
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(geometryHost.sampledPositions(), QVariantList({0, 6, 13}));
    QCOMPARE(geometryHost.sampledSourcePositions(), QVariantList({0, 6, 13}));
    QCOMPARE(entries.at(0).toMap().value(QStringLiteral("y")).toReal(), 5.0);
    QCOMPARE(entries.at(1).toMap().value(QStringLiteral("y")).toReal(), 17.0);
    QCOMPARE(entries.at(2).toMap().value(QStringLiteral("y")).toReal(), 31.0);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_keepsSingleRawPhysicalLineAsOneEditorLine()
{
    ContentsGutterLineNumberGeometry geometry;

    const QString firstParagraph = QStringLiteral("first visible paragraph");
    const QString secondParagraph = QStringLiteral("second visible paragraph");
    const QString resourceText = QStringLiteral("<resource type=\"image\" path=\"cover.png\" />");
    const QString sourceText = resourceText + firstParagraph + secondParagraph;
    const int resourceEnd = resourceText.size();
    const int firstParagraphStart = resourceEnd;
    const int firstParagraphEnd = firstParagraphStart + firstParagraph.size();
    const int secondParagraphStart = firstParagraphEnd;

    QVariantMap resourceBlock;
    resourceBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));
    resourceBlock.insert(QStringLiteral("sourceStart"), 0);
    resourceBlock.insert(QStringLiteral("sourceEnd"), resourceEnd);

    QVariantMap firstBlock;
    firstBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    firstBlock.insert(QStringLiteral("sourceStart"), firstParagraphStart);
    firstBlock.insert(QStringLiteral("sourceEnd"), firstParagraphEnd);
    firstBlock.insert(QStringLiteral("sourceText"), firstParagraph);
    firstBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap secondBlock;
    secondBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    secondBlock.insert(QStringLiteral("sourceStart"), secondParagraphStart);
    secondBlock.insert(QStringLiteral("sourceEnd"), sourceText.size());
    secondBlock.insert(QStringLiteral("sourceText"), secondParagraph);
    secondBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap visibleTextGroup;
    visibleTextGroup.insert(QStringLiteral("type"), QStringLiteral("text-group"));
    visibleTextGroup.insert(QStringLiteral("flattenedInteractiveGroup"), true);
    visibleTextGroup.insert(QStringLiteral("sourceStart"), firstParagraphStart);
    visibleTextGroup.insert(QStringLiteral("sourceEnd"), sourceText.size());
    visibleTextGroup.insert(
        QStringLiteral("sourceText"),
        firstParagraph + QLatin1Char('\n') + secondParagraph);
    visibleTextGroup.insert(QStringLiteral("logicalLineCountHint"), 2);
    visibleTextGroup.insert(QStringLiteral("groupedBlocks"), QVariantList{firstBlock, secondBlock});

    geometry.setSourceText(sourceText);
    geometry.setDisplayBlocks(QVariantList{resourceBlock, visibleTextGroup});
    geometry.setLineNumberCount(1);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(0.0);
    geometry.setFallbackLineHeight(20.0);

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).toMap().value(QStringLiteral("lineNumber")).toInt(), 1);
    QCOMPARE(entries.at(0).toMap().value(QStringLiteral("blockType")).toString(), QStringLiteral("resource"));
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_mergesDisplayAndStructuredBlockStreams()
{
    ContentsGutterLineNumberGeometry geometry;

    const QString resourceText = QStringLiteral("<resource type=\"image\" path=\"cover.png\" />");
    const QString firstParagraph = QStringLiteral("first structured paragraph");
    const QString secondParagraph = QStringLiteral("second structured paragraph");
    const QString sourceText = resourceText
        + QLatin1Char('\n')
        + firstParagraph
        + QLatin1Char('\n')
        + secondParagraph;
    const int resourceEnd = resourceText.size();
    const int firstParagraphStart = resourceEnd + 1;
    const int firstParagraphEnd = firstParagraphStart + firstParagraph.size();
    const int secondParagraphStart = firstParagraphEnd + 1;

    QVariantMap visibleResourceBlock;
    visibleResourceBlock.insert(QStringLiteral("renderDelegateType"), QStringLiteral("resource"));
    visibleResourceBlock.insert(QStringLiteral("sourceStart"), 0);
    visibleResourceBlock.insert(QStringLiteral("sourceEnd"), resourceEnd);

    QVariantMap firstStructuredBlock;
    firstStructuredBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    firstStructuredBlock.insert(QStringLiteral("sourceStart"), firstParagraphStart);
    firstStructuredBlock.insert(QStringLiteral("sourceEnd"), firstParagraphEnd);
    firstStructuredBlock.insert(QStringLiteral("sourceText"), firstParagraph);
    firstStructuredBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap secondStructuredBlock;
    secondStructuredBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    secondStructuredBlock.insert(QStringLiteral("sourceStart"), secondParagraphStart);
    secondStructuredBlock.insert(QStringLiteral("sourceEnd"), sourceText.size());
    secondStructuredBlock.insert(QStringLiteral("sourceText"), secondParagraph);
    secondStructuredBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    geometry.setSourceText(sourceText);
    geometry.setDisplayBlocks(QVariantList{visibleResourceBlock});
    geometry.setDocumentBlocks(QVariantList{firstStructuredBlock, secondStructuredBlock});
    geometry.setLineNumberCount(1);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(0.0);
    geometry.setFallbackLineHeight(20.0);

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(entries.at(0).toMap().value(QStringLiteral("blockType")).toString(), QStringLiteral("resource"));
    QCOMPARE(entries.at(1).toMap().value(QStringLiteral("blockType")).toString(), QStringLiteral("paragraph"));
    QCOMPARE(entries.at(2).toMap().value(QStringLiteral("lineNumber")).toInt(), 3);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_ignoresEditorContentHeightWithoutGeometry()
{
    ContentsGutterLineNumberGeometry geometry;

    const QString sourceText = QStringLiteral(
        "<resource type=\"image\" path=\"cover.png\" />\n"
        "second raw line\n"
        "third raw line");
    const int resourceEnd = sourceText.indexOf(QLatin1Char('\n'));
    const int secondStart = resourceEnd + 1;
    const int secondEnd = sourceText.indexOf(QLatin1Char('\n'), secondStart);
    const int thirdStart = secondEnd + 1;

    QVariantMap resourceBlock;
    resourceBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));
    resourceBlock.insert(QStringLiteral("sourceStart"), 0);
    resourceBlock.insert(QStringLiteral("sourceEnd"), resourceEnd);

    QVariantMap secondBlock;
    secondBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    secondBlock.insert(QStringLiteral("sourceStart"), secondStart);
    secondBlock.insert(QStringLiteral("sourceEnd"), secondEnd);
    secondBlock.insert(
        QStringLiteral("sourceText"),
        sourceText.mid(secondStart, secondEnd - secondStart));
    secondBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap thirdBlock;
    thirdBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    thirdBlock.insert(QStringLiteral("sourceStart"), thirdStart);
    thirdBlock.insert(QStringLiteral("sourceEnd"), sourceText.size());
    thirdBlock.insert(QStringLiteral("sourceText"), sourceText.mid(thirdStart));
    thirdBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    geometry.setSourceText(sourceText);
    geometry.setDocumentBlocks(QVariantList{resourceBlock, secondBlock, thirdBlock});
    geometry.setLineNumberCount(3);
    geometry.setFallbackTopInset(0.0);
    geometry.setFallbackLineHeight(20.0);
    geometry.setEditorContentHeight(120.0);
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);

    const QVariantMap resourceEntry = entries.at(0).toMap();
    const QVariantMap secondEntry = entries.at(1).toMap();
    const QVariantMap thirdEntry = entries.at(2).toMap();
    QCOMPARE(resourceEntry.value(QStringLiteral("lineNumber")).toInt(), 1);
    QCOMPARE(resourceEntry.value(QStringLiteral("y")).toReal(), 0.0);
    QCOMPARE(resourceEntry.value(QStringLiteral("height")).toReal(), 20.0);
    QCOMPARE(secondEntry.value(QStringLiteral("lineNumber")).toInt(), 2);
    QCOMPARE(secondEntry.value(QStringLiteral("y")).toReal(), 20.0);
    QCOMPARE(thirdEntry.value(QStringLiteral("lineNumber")).toInt(), 3);
    QCOMPARE(thirdEntry.value(QStringLiteral("y")).toReal(), 40.0);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_usesEditorGeometryDeltaForTallResourceRows()
{
    ContentsGutterLineNumberGeometry geometry;
    FakeGutterDisplayGeometryHost geometryHost;
    const QString sourceText = QStringLiteral(
        "first\n"
        "<resource type=\"image\" path=\"cover.png\" />\n"
        "third");
    const int firstEnd = sourceText.indexOf(QLatin1Char('\n'));
    const int resourceStart = sourceText.indexOf(QStringLiteral("<resource"));
    const int resourceEnd = sourceText.indexOf(QLatin1Char('\n'), resourceStart);
    const int thirdStart = resourceEnd + 1;

    QVariantMap firstBlock;
    firstBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    firstBlock.insert(QStringLiteral("sourceStart"), 0);
    firstBlock.insert(QStringLiteral("sourceEnd"), firstEnd);
    firstBlock.insert(QStringLiteral("sourceText"), sourceText.left(firstEnd));
    firstBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap resourceBlock;
    resourceBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));
    resourceBlock.insert(QStringLiteral("sourceStart"), resourceStart);
    resourceBlock.insert(QStringLiteral("sourceEnd"), resourceEnd);

    QVariantMap thirdBlock;
    thirdBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    thirdBlock.insert(QStringLiteral("sourceStart"), thirdStart);
    thirdBlock.insert(QStringLiteral("sourceEnd"), sourceText.size());
    thirdBlock.insert(QStringLiteral("sourceText"), sourceText.mid(thirdStart));
    thirdBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    geometry.setSourceText(sourceText);
    geometry.setDocumentBlocks(QVariantList{firstBlock, resourceBlock, thirdBlock});
    geometry.setEditorGeometryHost(&geometryHost);
    geometry.setLineNumberCount(3);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(0.0);
    geometry.setFallbackLineHeight(20.0);
    geometry.setEditorContentHeight(108.0);
    geometryHost.setSourceYOverride(0, 0.0);
    geometryHost.setSourceYOverride(resourceStart, 20.0);
    geometryHost.setSourceYOverride(thirdStart, 88.0);
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);

    const QVariantMap firstEntry = entries.at(0).toMap();
    const QVariantMap resourceEntry = entries.at(1).toMap();
    const QVariantMap thirdEntry = entries.at(2).toMap();
    QCOMPARE(firstEntry.value(QStringLiteral("y")).toReal(), 0.0);
    QCOMPARE(firstEntry.value(QStringLiteral("height")).toReal(), 20.0);
    QCOMPARE(resourceEntry.value(QStringLiteral("y")).toReal(), 20.0);
    QCOMPARE(resourceEntry.value(QStringLiteral("height")).toReal(), 68.0);
    QCOMPARE(thirdEntry.value(QStringLiteral("y")).toReal(), 88.0);
    QCOMPARE(thirdEntry.value(QStringLiteral("height")).toReal(), 20.0);
    QVERIFY(resourceEntry.value(QStringLiteral("height")).toReal()
            > firstEntry.value(QStringLiteral("height")).toReal());
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_ignoresRenderedResourceMetadataWithoutGeometry()
{
    ContentsGutterLineNumberGeometry geometry;

    const QString sourceText = QStringLiteral(
        "alpha\n"
        "<resource type=\"image\" path=\"cover.png\" />\n"
        "<resource type=\"document\" path=\"brief.pdf\" />\n"
        "omega");
    const int imageStart = sourceText.indexOf(QStringLiteral("<resource type=\"image\""));
    const int imageEnd = sourceText.indexOf(QLatin1Char('\n'), imageStart);
    const int documentStart = sourceText.indexOf(QStringLiteral("<resource type=\"document\""));
    const int documentEnd = sourceText.indexOf(QLatin1Char('\n'), documentStart);
    const int omegaStart = sourceText.indexOf(QStringLiteral("omega"));
    const int alphaEnd = sourceText.indexOf(QLatin1Char('\n'));

    QVariantMap alphaBlock;
    alphaBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    alphaBlock.insert(QStringLiteral("sourceStart"), 0);
    alphaBlock.insert(QStringLiteral("sourceEnd"), alphaEnd);
    alphaBlock.insert(QStringLiteral("sourceText"), sourceText.left(alphaEnd));
    alphaBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap imageBlock;
    imageBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));
    imageBlock.insert(QStringLiteral("resourceIndex"), 0);
    imageBlock.insert(QStringLiteral("sourceStart"), imageStart);
    imageBlock.insert(QStringLiteral("sourceEnd"), imageEnd);

    QVariantMap documentBlock;
    documentBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));
    documentBlock.insert(QStringLiteral("resourceIndex"), 1);
    documentBlock.insert(QStringLiteral("sourceStart"), documentStart);
    documentBlock.insert(QStringLiteral("sourceEnd"), documentEnd);

    QVariantMap omegaBlock;
    omegaBlock.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
    omegaBlock.insert(QStringLiteral("sourceStart"), omegaStart);
    omegaBlock.insert(QStringLiteral("sourceEnd"), sourceText.size());
    omegaBlock.insert(QStringLiteral("sourceText"), sourceText.mid(omegaStart));
    omegaBlock.insert(QStringLiteral("logicalLineCountHint"), 1);

    QVariantMap renderedImage;
    renderedImage.insert(QStringLiteral("index"), 0);
    renderedImage.insert(QStringLiteral("renderMode"), QStringLiteral("image"));
    renderedImage.insert(QStringLiteral("sourceStart"), imageStart);
    renderedImage.insert(QStringLiteral("sourceEnd"), imageEnd);
    renderedImage.insert(QStringLiteral("imageWidth"), 11);
    renderedImage.insert(QStringLiteral("imageHeight"), 7);

    QVariantMap renderedDocument;
    renderedDocument.insert(QStringLiteral("index"), 1);
    renderedDocument.insert(QStringLiteral("renderMode"), QStringLiteral("document"));
    renderedDocument.insert(QStringLiteral("sourceStart"), documentStart);
    renderedDocument.insert(QStringLiteral("sourceEnd"), documentEnd);

    geometry.setSourceText(sourceText);
    geometry.setDocumentBlocks(QVariantList{alphaBlock, imageBlock, documentBlock, omegaBlock});
    geometry.setRenderedResources(QVariantList{renderedImage, renderedDocument});
    geometry.setLineNumberCount(4);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackLineHeight(20.0);
    geometry.setEditorContentHeight(132.0);
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 4);

    const QVariantMap imageEntry = entries.at(1).toMap();
    const QVariantMap documentEntry = entries.at(2).toMap();
    const QVariantMap omegaEntry = entries.at(3).toMap();
    QCOMPARE(imageEntry.value(QStringLiteral("y")).toReal(), 20.0);
    QCOMPARE(imageEntry.value(QStringLiteral("height")).toReal(), 20.0);
    QCOMPARE(documentEntry.value(QStringLiteral("y")).toReal(), 40.0);
    QCOMPARE(documentEntry.value(QStringLiteral("height")).toReal(), 20.0);
    QCOMPARE(omegaEntry.value(QStringLiteral("y")).toReal(), 60.0);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_ignoresLogicalLineOffsetsForSourceLineRowCount()
{
    ContentsGutterLineNumberGeometry geometry;
    FakeGutterDisplayGeometryHost geometryHost;
    QObject gutterTarget;

    geometry.setEditorGeometryHost(&geometryHost);
    geometry.setMapTarget(&gutterTarget);
    geometry.setSourceText(QStringLiteral("first\nsecond\nthird"));
    geometry.setLogicalLineStartOffsets(QVariantList{0, 0, 12});
    geometry.setLineNumberCount(3);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(0.0);
    geometry.setFallbackLineHeight(20.0);
    geometryHost.clearSampledPositions();
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(geometryHost.sampledPositions(), QVariantList({0, 6, 13}));
    QCOMPARE(geometryHost.sampledSourcePositions(), QVariantList({0, 6, 13}));

    const qreal firstY = entries.at(0).toMap().value(QStringLiteral("y")).toReal();
    const qreal secondY = entries.at(1).toMap().value(QStringLiteral("y")).toReal();
    const qreal thirdY = entries.at(2).toMap().value(QStringLiteral("y")).toReal();
    QCOMPARE(firstY, 8.0);
    QCOMPARE(secondY, 20.0);
    QCOMPARE(thirdY, 34.0);
    QVERIFY(secondY > firstY);
    QVERIFY(thirdY > secondY);
}

void WhatSonCppRegressionTests::contentsGutterMarkerGeometry_marksCursorAndUnsavedLineSpans()
{
    QVariantList lineNumberEntries;
    QVariantMap firstLine;
    firstLine.insert(QStringLiteral("lineNumber"), 1);
    firstLine.insert(QStringLiteral("y"), 10.0);
    lineNumberEntries.append(firstLine);
    QVariantMap secondLine;
    secondLine.insert(QStringLiteral("lineNumber"), 2);
    secondLine.insert(QStringLiteral("y"), 30.0);
    lineNumberEntries.append(secondLine);
    QVariantMap thirdLine;
    thirdLine.insert(QStringLiteral("lineNumber"), 3);
    thirdLine.insert(QStringLiteral("y"), 50.0);
    lineNumberEntries.append(thirdLine);

    ContentsGutterMarkerGeometry geometry;
    geometry.setEditorMounted(true);
    geometry.setLineNumberBaseOffset(1);
    geometry.setLineNumberEntries(lineNumberEntries);
    geometry.setMarkerHeight(12.0);
    geometry.setSavedSourceText(QStringLiteral("alpha\nbeta\ngamma"));
    geometry.setSourceText(QStringLiteral("alpha\nBETA\nGAMMA"));
    geometry.setCursorPosition(QStringLiteral("alpha\nBETA").size());

    const QVariantList entries = geometry.markerEntries();
    QCOMPARE(entries.size(), 2);

    const QVariantMap unsavedEntry = entries.at(0).toMap();
    QCOMPARE(unsavedEntry.value(QStringLiteral("type")).toString(), QStringLiteral("unsaved"));
    QCOMPARE(unsavedEntry.value(QStringLiteral("lineNumber")).toInt(), 2);
    QCOMPARE(unsavedEntry.value(QStringLiteral("lineSpan")).toInt(), 2);
    QCOMPARE(unsavedEntry.value(QStringLiteral("y")).toReal(), 30.0);
    QCOMPARE(unsavedEntry.value(QStringLiteral("height")).toReal(), 32.0);

    const QVariantMap cursorEntry = entries.at(1).toMap();
    QCOMPARE(cursorEntry.value(QStringLiteral("type")).toString(), QStringLiteral("cursor"));
    QCOMPARE(cursorEntry.value(QStringLiteral("lineNumber")).toInt(), 2);
    QCOMPARE(cursorEntry.value(QStringLiteral("lineSpan")).toInt(), 1);
    QCOMPARE(cursorEntry.value(QStringLiteral("y")).toReal(), 30.0);
    QCOMPARE(cursorEntry.value(QStringLiteral("height")).toReal(), 12.0);
    QCOMPARE(geometry.cursorLineNumber(), 2);

    geometry.setEditorMounted(false);
    QCOMPARE(geometry.markerEntries().size(), 0);
    QCOMPARE(geometry.cursorLineNumber(), -1);

    ContentsGutterMarkerGeometry deferredSourceGeometry;
    deferredSourceGeometry.setEditorMounted(true);
    deferredSourceGeometry.setLineNumberBaseOffset(1);
    deferredSourceGeometry.setLineNumberEntries(lineNumberEntries);
    deferredSourceGeometry.setMarkerHeight(12.0);
    deferredSourceGeometry.setCursorPosition(100);
    deferredSourceGeometry.setSourceText(QStringLiteral("alpha\nbeta\ngamma"));
    QCOMPARE(deferredSourceGeometry.cursorLineNumber(), 3);
}

void WhatSonCppRegressionTests::contentsMinimapLayoutMetrics_resolvesRuntimeVisibilityAndDesignRows()
{
    ContentsMinimapLayoutMetrics metrics;
    metrics.setGapNone(0);
    metrics.setGap8(8);
    metrics.setGap12(12);
    metrics.setGap20(20);
    metrics.setGap24(24);
    metrics.setStrokeThin(1);
    metrics.setButtonMinWidth(84);

    QCOMPARE(metrics.defaultMinimapWidth(), 84);
    QCOMPARE(metrics.designRowCount(), 64);
    QCOMPARE(metrics.effectiveMinimapWidth(), 84);

    metrics.setLogicalLineCount(0);
    QCOMPARE(metrics.effectiveRowCount(), 1);

    metrics.setLogicalLineCount(73);
    QCOMPARE(metrics.effectiveRowCount(), 73);

    metrics.setMinimapVisible(false);
    QCOMPARE(metrics.effectiveMinimapWidth(), 0);
}
