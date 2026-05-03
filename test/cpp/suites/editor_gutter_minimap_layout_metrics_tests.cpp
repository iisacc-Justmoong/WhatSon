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

    geometry.setLineNumberCount(0);
    QCOMPARE(geometry.lineNumberEntries().size(), 1);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_samplesVisibleDisplayOffsets()
{
    ContentsGutterLineNumberGeometry geometry;
    FakeGutterDisplayGeometryHost geometryHost;
    QObject gutterTarget;
    QVariantList logicalToSourceOffsets;
    for (int index = 0; index <= 24; ++index)
    {
        logicalToSourceOffsets.append(index);
    }
    logicalToSourceOffsets.replace(0, 7);
    logicalToSourceOffsets.replace(12, 31);
    logicalToSourceOffsets.replace(24, 52);

    geometry.setEditorGeometryHost(&geometryHost);
    geometry.setMapTarget(&gutterTarget);
    geometry.setSourceText(QString(80, QLatin1Char('x')));
    geometry.setLogicalLineStartOffsets(QVariantList{0, 12, 24});
    geometry.setLogicalToSourceOffsets(logicalToSourceOffsets);
    geometry.setLineNumberCount(3);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(80.0);
    geometry.setFallbackLineHeight(20.0);
    geometryHost.clearSampledPositions();
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(geometryHost.sampledPositions(), QVariantList({0, 12, 24}));
    QCOMPARE(geometryHost.sampledSourcePositions(), QVariantList({7, 31, 52}));
    QCOMPARE(entries.at(0).toMap().value(QStringLiteral("y")).toReal(), 8.0);
    QCOMPARE(entries.at(1).toMap().value(QStringLiteral("y")).toReal(), 32.0);
    QCOMPARE(entries.at(2).toMap().value(QStringLiteral("y")).toReal(), 56.0);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_expandsResourceRowsToRenderedContentHeight()
{
    ContentsGutterLineNumberGeometry geometry;
    FakeGutterDisplayGeometryHost geometryHost;
    QObject gutterTarget;
    QVariantList logicalToSourceOffsets;
    for (int index = 0; index <= 24; ++index)
    {
        logicalToSourceOffsets.append(index);
    }
    logicalToSourceOffsets.replace(0, 7);
    logicalToSourceOffsets.replace(12, 31);
    logicalToSourceOffsets.replace(24, 52);

    QVariantMap resourceBlock;
    resourceBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));
    resourceBlock.insert(QStringLiteral("sourceStart"), 31);
    resourceBlock.insert(QStringLiteral("sourceEnd"), 50);

    geometry.setEditorGeometryHost(&geometryHost);
    geometry.setMapTarget(&gutterTarget);
    geometry.setSourceText(QString(80, QLatin1Char('x')));
    geometry.setDocumentBlocks(QVariantList{resourceBlock});
    geometry.setLogicalLineStartOffsets(QVariantList{0, 12, 24});
    geometry.setLogicalToSourceOffsets(logicalToSourceOffsets);
    geometry.setLineNumberCount(3);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackTopInset(80.0);
    geometry.setFallbackLineHeight(20.0);
    geometry.setEditorContentHeight(108.0);
    geometryHost.clearSampledPositions();
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(geometryHost.sampledPositions(), QVariantList({0, 12, 24}));
    QCOMPARE(geometryHost.sampledSourcePositions(), QVariantList({7, 31, 52}));

    const QVariantMap firstEntry = entries.at(0).toMap();
    const QVariantMap resourceEntry = entries.at(1).toMap();
    const QVariantMap thirdEntry = entries.at(2).toMap();
    QCOMPARE(firstEntry.value(QStringLiteral("y")).toReal(), 8.0);
    QCOMPARE(firstEntry.value(QStringLiteral("height")).toReal(), 24.0);
    QCOMPARE(resourceEntry.value(QStringLiteral("y")).toReal(), 32.0);
    QCOMPARE(resourceEntry.value(QStringLiteral("height")).toReal(), 64.0);
    QCOMPARE(thirdEntry.value(QStringLiteral("y")).toReal(), 96.0);
    QCOMPARE(thirdEntry.value(QStringLiteral("height")).toReal(), 20.0);
    QVERIFY(resourceEntry.value(QStringLiteral("height")).toReal()
            > firstEntry.value(QStringLiteral("height")).toReal());
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_assignsRenderedResourceHeightToVisibleImageRows()
{
    ContentsGutterLineNumberGeometry geometry;
    FakeGutterDisplayGeometryHost geometryHost;
    QObject gutterTarget;

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

    QVariantList logicalToSourceOffsets;
    for (int index = 0; index <= 36; ++index)
    {
        logicalToSourceOffsets.append(index);
    }
    logicalToSourceOffsets.replace(0, 0);
    logicalToSourceOffsets.replace(12, imageStart);
    logicalToSourceOffsets.replace(24, documentStart);
    logicalToSourceOffsets.replace(36, omegaStart);

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

    geometry.setEditorGeometryHost(&geometryHost);
    geometry.setMapTarget(&gutterTarget);
    geometry.setSourceText(sourceText);
    geometry.setDocumentBlocks(QVariantList{imageBlock, documentBlock});
    geometry.setRenderedResources(QVariantList{renderedImage, renderedDocument});
    geometry.setLogicalLineStartOffsets(QVariantList{0, 12, 24, 36});
    geometry.setLogicalToSourceOffsets(logicalToSourceOffsets);
    geometry.setLineNumberCount(4);
    geometry.setLineNumberBaseOffset(1);
    geometry.setFallbackLineHeight(20.0);
    geometry.setEditorContentHeight(132.0);
    geometryHost.clearSampledPositions();
    geometry.refresh();

    const QVariantList entries = geometry.lineNumberEntries();
    QCOMPARE(entries.size(), 4);

    const QVariantMap imageEntry = entries.at(1).toMap();
    const QVariantMap documentEntry = entries.at(2).toMap();
    const QVariantMap omegaEntry = entries.at(3).toMap();
    QCOMPARE(imageEntry.value(QStringLiteral("y")).toReal(), 32.0);
    QCOMPARE(imageEntry.value(QStringLiteral("height")).toReal(), 64.0);
    QCOMPARE(documentEntry.value(QStringLiteral("y")).toReal(), 96.0);
    QCOMPARE(documentEntry.value(QStringLiteral("height")).toReal(), 24.0);
    QCOMPARE(omegaEntry.value(QStringLiteral("y")).toReal(), 120.0);
}

void WhatSonCppRegressionTests::contentsGutterLineNumberGeometry_separatesDuplicateDisplaySamples()
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
    QCOMPARE(geometryHost.sampledPositions(), QVariantList({0, 0, 12}));

    const qreal firstY = entries.at(0).toMap().value(QStringLiteral("y")).toReal();
    const qreal secondY = entries.at(1).toMap().value(QStringLiteral("y")).toReal();
    const qreal thirdY = entries.at(2).toMap().value(QStringLiteral("y")).toReal();
    QCOMPARE(firstY, 8.0);
    QCOMPARE(secondY, 28.0);
    QCOMPARE(thirdY, 32.0);
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
