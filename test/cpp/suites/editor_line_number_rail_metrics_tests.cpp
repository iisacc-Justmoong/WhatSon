#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsLineNumberRailMetrics_buildsRowsFromLogicalBlocks()
{
    const QString headerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/lineNumber/ContentsLineNumberRailMetrics.hpp"));
    const QString implementationSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/lineNumber/ContentsLineNumberRailMetrics.cpp"));

    QVERIFY(headerSource.contains(QStringLiteral("geometryRows")));
    QVERIFY(!headerSource.contains(QStringLiteral("textGeometryItem")));
    QVERIFY(!headerSource.contains(QStringLiteral("resourceGeometryItem")));
    QVERIFY(!headerSource.contains(QStringLiteral("targetItem")));
    QVERIFY(!headerSource.contains(QStringLiteral("geometryProvider")));
    QVERIFY(!implementationSource.contains(QStringLiteral("IContentsEditorGeometryProvider")));
    QVERIFY(!implementationSource.contains(QStringLiteral("positionToRectangle")));
    QVERIFY(!implementationSource.contains(QStringLiteral("QQuickItem")));

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

void WhatSonCppRegressionTests::contentsLineNumberRailMetrics_mapsRowsFromWholeLogicalText()
{
    const QString sourceText =
        QStringLiteral("<title>Alpha</title>\n\n<paragraph>Beta</paragraph>");
    ContentsLogicalTextBridge logicalTextBridge;
    logicalTextBridge.setText(sourceText);
    QCOMPARE(logicalTextBridge.logicalText(), QStringLiteral("Alpha\n\nBeta"));

    ContentsLineNumberRailMetrics metrics;
    metrics.setTextLineHeight(18.0);
    metrics.setGeometryWidth(320.0);
    metrics.setSourceText(sourceText);
    metrics.setLogicalText(logicalTextBridge.logicalText());
    metrics.setLogicalToSourceOffsets(logicalTextBridge.logicalToSourceOffsets());

    const int alphaStart = sourceText.indexOf(QStringLiteral("Alpha"));
    const int betaStart = sourceText.indexOf(QStringLiteral("Beta"));
    metrics.setNormalizedHtmlBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("htmlTokenStartIndex"), 0},
            {QStringLiteral("logicalLineCountHint"), 1},
            {QStringLiteral("sourceEnd"), alphaStart + 5},
            {QStringLiteral("sourceStart"), alphaStart},
            {QStringLiteral("sourceText"), QStringLiteral("Alpha")},
        },
        QVariantMap{
            {QStringLiteral("htmlTokenStartIndex"), 1},
            {QStringLiteral("logicalLineCountHint"), 1},
            {QStringLiteral("sourceEnd"), betaStart + 4},
            {QStringLiteral("sourceStart"), betaStart},
            {QStringLiteral("sourceText"), QStringLiteral("Beta")},
        },
    });
    metrics.setGeometryRows(QVariantList{
        QVariantMap{
            {QStringLiteral("geometryAvailable"), true},
            {QStringLiteral("height"), 18.0},
            {QStringLiteral("y"), 0.0},
        },
        QVariantMap{
            {QStringLiteral("geometryAvailable"), true},
            {QStringLiteral("height"), 18.0},
            {QStringLiteral("y"), 54.0},
        },
        QVariantMap{
            {QStringLiteral("geometryAvailable"), true},
            {QStringLiteral("height"), 18.0},
            {QStringLiteral("y"), 72.0},
        },
    });

    const QVariantList ranges = metrics.logicalLineRanges();
    QCOMPARE(ranges.size(), 3);
    QCOMPARE(ranges.at(0).toMap().value(QStringLiteral("number")).toInt(), 1);
    QCOMPARE(ranges.at(0).toMap().value(QStringLiteral("logicalStart")).toInt(), 0);
    QCOMPARE(ranges.at(0).toMap().value(QStringLiteral("logicalEnd")).toInt(), 5);
    QCOMPARE(ranges.at(1).toMap().value(QStringLiteral("number")).toInt(), 2);
    QCOMPARE(ranges.at(1).toMap().value(QStringLiteral("logicalStart")).toInt(), 6);
    QCOMPARE(ranges.at(1).toMap().value(QStringLiteral("logicalEnd")).toInt(), 6);
    QCOMPARE(ranges.at(2).toMap().value(QStringLiteral("number")).toInt(), 3);
    QCOMPARE(ranges.at(2).toMap().value(QStringLiteral("logicalStart")).toInt(), 7);
    QVERIFY(ranges.at(1).toMap().value(QStringLiteral("sourceStart")).toInt()
            < ranges.at(2).toMap().value(QStringLiteral("sourceStart")).toInt());

    const QVariantList rows = metrics.rows();
    QCOMPARE(rows.size(), 3);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("number")).toInt(), 2);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("y")).toDouble(), 54.0);
    QCOMPARE(rows.at(2).toMap().value(QStringLiteral("y")).toDouble(), 72.0);
}
