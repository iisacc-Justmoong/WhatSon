#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsEditorVisualLineMetrics_expandsTallVisualBlocks()
{
    const QString headerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp"));
    const QString implementationSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.cpp"));

    QVERIFY(!headerSource.contains(QStringLiteral("QObject* textItem")));
    QVERIFY(!headerSource.contains(QStringLiteral("QPointer")));
    QVERIFY(!implementationSource.contains(QStringLiteral("positionAt")));
    QVERIFY(!implementationSource.contains(QStringLiteral("positionToRectangle")));

    ContentsEditorVisualLineMetrics metrics;
    metrics.setMeasuredVisualLineCount(5);

    QCOMPARE(metrics.visualLineCount(), 5);

    const QVariantList fallbackRatios = metrics.visualLineWidthRatios();
    QCOMPARE(fallbackRatios.size(), 5);
    for (const QVariant& ratio : fallbackRatios)
    {
        QCOMPARE(ratio.toDouble(), 1.0);
    }

    metrics.setMeasuredLineWidthRatios(QVariantList{0.25, 0.5});
    QCOMPARE(metrics.visualLineCount(), 5);
    const QVariantList measuredRatios = metrics.visualLineWidthRatios();
    QCOMPARE(measuredRatios.at(0).toDouble(), 0.25);
    QCOMPARE(measuredRatios.at(1).toDouble(), 0.5);
    QCOMPARE(measuredRatios.at(2).toDouble(), 1.0);
}
