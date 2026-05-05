#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsEditorVisualLineMetrics_expandsTallVisualBlocks()
{
    ContentsEditorVisualLineMetrics metrics;
    metrics.setFallbackWidth(120.0);
    metrics.setLineHeight(20.0);
    metrics.setStrokeThin(2.0);
    metrics.setTextLineCount(1);
    metrics.setTextContentHeight(100.0);

    QCOMPARE(metrics.visualLineCount(), 5);

    const QVariantList fallbackRatios = metrics.visualLineWidthRatios();
    QCOMPARE(fallbackRatios.size(), 5);
    for (const QVariant& ratio : fallbackRatios)
    {
        QCOMPARE(ratio.toDouble(), 1.0);
    }
}
