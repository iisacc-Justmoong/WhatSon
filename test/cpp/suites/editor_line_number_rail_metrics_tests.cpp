#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QQmlComponent>

namespace
{
    std::unique_ptr<QObject> createGeometryProbe(
        QQmlEngine& engine,
        const int length,
        const qreal contentHeight,
        const QVariantMap& rectangles)
    {
        QQmlComponent component(&engine);
        component.setData(
            QByteArrayLiteral(R"QML(
import QtQml

QtObject {
    property int length: 0
    property real contentHeight: 0
    property var rectangles: ({})

    function positionToRectangle(position: int): rect {
        const key = String(Math.max(0, Number(position) || 0));
        if (rectangles[key] !== undefined)
            return rectangles[key];
        return Qt.rect(0, 0, 0, 18);
    }
}
)QML"),
            QUrl());
        if (component.status() == QQmlComponent::Error)
        {
            qWarning().noquote() << component.errorString();
            return nullptr;
        }

        std::unique_ptr<QObject> object(component.create());
        if (object == nullptr)
        {
            qWarning().noquote() << component.errorString();
            return nullptr;
        }

        object->setProperty("length", length);
        object->setProperty("contentHeight", contentHeight);
        object->setProperty("rectangles", rectangles);
        return std::move(object);
    }
} // namespace

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

void WhatSonCppRegressionTests::contentsLineNumberRailMetrics_keepsResourceRowsIndependent()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" path=\"cover.png\" />");
    const QString sourceText = QStringLiteral("alpha\n") + resourceTag + QStringLiteral("\nbeta");

    ContentsLogicalTextBridge logicalTextBridge;
    logicalTextBridge.setText(sourceText);
    QCOMPARE(
        logicalTextBridge.logicalText(),
        QStringLiteral("alpha\n") + QString(1, QChar(0xfffc)) + QStringLiteral("\nbeta"));

    const int resourceStart = QStringLiteral("alpha\n").size();
    const int resourceEnd = resourceStart + resourceTag.size();

    ContentsLineNumberRailMetrics metrics;
    metrics.setTextLineHeight(18.0);
    metrics.setGeometryWidth(320.0);
    metrics.setSourceText(sourceText);
    metrics.setLogicalText(logicalTextBridge.logicalText());
    metrics.setNormalizedHtmlBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("htmlBlockIsDisplayBlock"), true},
            {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
            {QStringLiteral("htmlTokenStartIndex"), 1},
            {QStringLiteral("logicalLineCountHint"), 1},
            {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
            {QStringLiteral("sourceEnd"), resourceEnd},
            {QStringLiteral("sourceStart"), resourceStart},
            {QStringLiteral("sourceText"), resourceTag},
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
            {QStringLiteral("height"), 420.0},
            {QStringLiteral("y"), 18.0},
        },
        QVariantMap{
            {QStringLiteral("geometryAvailable"), true},
            {QStringLiteral("height"), 18.0},
            {QStringLiteral("y"), 54.0},
        },
    });

    const QVariantList ranges = metrics.logicalLineRanges();
    QCOMPARE(ranges.size(), 3);
    QVERIFY(ranges.at(1).toMap().value(QStringLiteral("resourceRange")).toBool());

    const QVariantList rows = metrics.rows();
    QCOMPARE(rows.size(), 3);
    QCOMPARE(rows.at(0).toMap().value(QStringLiteral("y")).toDouble(), 0.0);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("y")).toDouble(), 18.0);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("height")).toDouble(), 18.0);
    QVERIFY(rows.at(1).toMap().value(QStringLiteral("resourceRange")).toBool());
    QCOMPARE(rows.at(2).toMap().value(QStringLiteral("y")).toDouble(), 54.0);
}

void WhatSonCppRegressionTests::contentsLineNumberRailMetrics_keepsResourceFramesSingleGutterLine()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" path=\"cover.png\" />");
    const QString sourceText = QStringLiteral("alpha\n") + resourceTag + QStringLiteral("\nbeta");

    ContentsLogicalTextBridge logicalTextBridge;
    logicalTextBridge.setText(sourceText);

    const int resourceStart = QStringLiteral("alpha\n").size();
    const int resourceEnd = resourceStart + resourceTag.size();

    ContentsLineNumberRailMetrics metrics;
    metrics.setTextLineHeight(18.0);
    metrics.setGeometryWidth(320.0);
    metrics.setSourceText(sourceText);
    metrics.setLogicalText(logicalTextBridge.logicalText());
    metrics.setNormalizedHtmlBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("htmlBlockIsDisplayBlock"), true},
            {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
            {QStringLiteral("htmlTokenStartIndex"), 1},
            {QStringLiteral("logicalLineCountHint"), 1},
            {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
            {QStringLiteral("sourceEnd"), resourceEnd},
            {QStringLiteral("sourceStart"), resourceStart},
            {QStringLiteral("sourceText"), resourceTag},
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
            {QStringLiteral("height"), 180.0},
            {QStringLiteral("y"), 18.0},
        },
        QVariantMap{
            {QStringLiteral("geometryAvailable"), true},
            {QStringLiteral("height"), 18.0},
            {QStringLiteral("y"), 216.0},
        },
    });

    const QVariantList rows = metrics.rows();
    QCOMPARE(rows.size(), 3);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("number")).toInt(), 2);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("y")).toDouble(), 18.0);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("height")).toDouble(), 18.0);
    QVERIFY(rows.at(1).toMap().value(QStringLiteral("resourceRange")).toBool());
    QCOMPARE(rows.at(2).toMap().value(QStringLiteral("number")).toInt(), 3);
    QCOMPARE(rows.at(2).toMap().value(QStringLiteral("y")).toDouble(), 216.0);
}

void WhatSonCppRegressionTests::contentsEditorGeometryProvider_capsMiddleResourceRowsAtNextMeasuredRow()
{
    QQmlEngine engine;
    const std::unique_ptr<QObject> textGeometry = createGeometryProbe(
        engine,
        11,
        0.0,
        QVariantMap{
            {QStringLiteral("0"), QRectF(0.0, 0.0, 120.0, 18.0)},
            {QStringLiteral("4"), QRectF(0.0, 0.0, 120.0, 18.0)},
            {QStringLiteral("7"), QRectF(0.0, 54.0, 120.0, 18.0)},
            {QStringLiteral("10"), QRectF(0.0, 54.0, 120.0, 18.0)},
        });
    QVERIFY(textGeometry != nullptr);

    const std::unique_ptr<QObject> resourceGeometry = createGeometryProbe(
        engine,
        11,
        720.0,
        QVariantMap{
            {QStringLiteral("6"), QRectF(0.0, 18.0, 120.0, 18.0)},
        });
    QVERIFY(resourceGeometry != nullptr);

    ContentsEditorGeometryProvider provider;
    provider.setFallbackLineHeight(18.0);
    provider.setFallbackWidth(320.0);
    provider.setLogicalLength(11);
    provider.setTextItem(textGeometry.get());
    provider.setResourceItem(resourceGeometry.get());
    provider.setLineNumberRanges(QVariantList{
        QVariantMap{
            {QStringLiteral("logicalEnd"), 5},
            {QStringLiteral("logicalStart"), 0},
            {QStringLiteral("resourceRange"), false},
        },
        QVariantMap{
            {QStringLiteral("logicalEnd"), 6},
            {QStringLiteral("logicalStart"), 6},
            {QStringLiteral("resourceRange"), true},
        },
        QVariantMap{
            {QStringLiteral("logicalEnd"), 11},
            {QStringLiteral("logicalStart"), 7},
            {QStringLiteral("resourceRange"), false},
        },
    });

    const QVariantList rows = provider.lineNumberGeometryRows();
    QCOMPARE(rows.size(), 3);
    QCOMPARE(rows.at(0).toMap().value(QStringLiteral("y")).toDouble(), 0.0);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("y")).toDouble(), 18.0);
    QCOMPARE(rows.at(1).toMap().value(QStringLiteral("height")).toDouble(), 36.0);
    QCOMPARE(rows.at(2).toMap().value(QStringLiteral("y")).toDouble(), 54.0);
}
