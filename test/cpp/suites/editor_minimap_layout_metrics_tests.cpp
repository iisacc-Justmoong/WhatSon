#include "test/cpp/whatson_cpp_regression_tests.hpp"

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

    metrics.setVisualLineCount(0);
    QCOMPARE(metrics.effectiveRowCount(), 1);

    metrics.setVisualLineCount(73);
    QCOMPARE(metrics.effectiveRowCount(), 73);

    metrics.setMinimapVisible(false);
    QCOMPARE(metrics.effectiveMinimapWidth(), 0);
}
