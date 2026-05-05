#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsLineNumberRailMetrics_buildsRowsFromLogicalBlocks()
{
    ContentsLineNumberRailMetrics metrics;
    metrics.setTextLineHeight(18.0);
    metrics.setGeometryWidth(320.0);
    metrics.setSourceText(QStringLiteral("alpha\nbeta\n<resource type=\"image\" path=\"cover.png\" />"));
    metrics.setLogicalText(QStringLiteral("alpha\nbeta\n"));

    const QVariantList blocks{
        QVariantMap{
            {QStringLiteral("htmlTokenStartIndex"), 0},
            {QStringLiteral("logicalLineCountHint"), 2},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceText"), QStringLiteral("alpha\nbeta")},
        },
        QVariantMap{
            {QStringLiteral("htmlTokenStartIndex"), 0},
            {QStringLiteral("logicalLineCountHint"), 2},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceText"), QStringLiteral("alpha\nbeta")},
        },
        QVariantMap{
            {QStringLiteral("htmlBlockIsDisplayBlock"), true},
            {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
            {QStringLiteral("htmlTokenStartIndex"), 1},
            {QStringLiteral("logicalLineCountHint"), 1},
            {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
            {QStringLiteral("sourceEnd"), 53},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceText"), QStringLiteral("<resource type=\"image\" path=\"cover.png\" />")},
        },
    };
    metrics.setNormalizedHtmlBlocks(blocks);

    const QVariantList ranges = metrics.logicalLineRanges();
    QCOMPARE(ranges.size(), 3);
    QCOMPARE(ranges.at(0).toMap().value(QStringLiteral("number")).toInt(), 1);
    QCOMPARE(ranges.at(0).toMap().value(QStringLiteral("sourceStart")).toInt(), 0);
    QCOMPARE(ranges.at(0).toMap().value(QStringLiteral("sourceEnd")).toInt(), 5);
    QCOMPARE(ranges.at(1).toMap().value(QStringLiteral("number")).toInt(), 2);
    QCOMPARE(ranges.at(1).toMap().value(QStringLiteral("sourceStart")).toInt(), 6);
    QCOMPARE(ranges.at(1).toMap().value(QStringLiteral("sourceEnd")).toInt(), 10);
    QCOMPARE(ranges.at(2).toMap().value(QStringLiteral("number")).toInt(), 3);
    QCOMPARE(ranges.at(2).toMap().value(QStringLiteral("sourceStart")).toInt(), 11);

    const QVariantList rows = metrics.rows();
    QCOMPARE(rows.size(), 3);
    QCOMPARE(rows.at(0).toMap().value(QStringLiteral("height")).toDouble(), 18.0);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("number")).toInt(), 2);
    QCOMPARE(rows.at(2).toMap().value(QStringLiteral("number")).toInt(), 3);
}
