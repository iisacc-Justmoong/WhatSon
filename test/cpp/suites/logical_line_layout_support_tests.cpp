#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::logicalLineLayoutSupport_mapsEditorRectanglesIntoBlockCoordinates()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/models/editor/structure/ContentsLogicalLineLayoutSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue buildEntries = library.property(QStringLiteral("buildEntries"));
    QVERIFY(buildEntries.isCallable());

    QJSValue editorItem = engine.newObject();
    editorItem.setProperty(QStringLiteral("width"), 200);
    editorItem.setProperty(
        QStringLiteral("positionToRectangle"),
        engine.evaluate(
            QStringLiteral(
                "(function (position) {"
                "  if (position <= 5)"
                "    return { x: position * 7, y: 0, height: 14 };"
                "  return { x: (position - 6) * 8, y: 18, height: 14 };"
                "})")));
    editorItem.setProperty(
        QStringLiteral("mapToItem"),
        engine.evaluate(
            QStringLiteral(
                "(function (_target, x, y) {"
                "  return { x: x + 4, y: y + 11 };"
                "})")));

    const QJSValue entries = buildEntries.call(QJSValueList {
        QJSValue(QStringLiteral("alpha\nbeta")),
        QJSValue(52),
        editorItem,
        engine.newObject(),
        QJSValue(12),
    });

    QVERIFY2(!entries.isError(), qPrintable(entries.toString()));
    QCOMPARE(entries.property(QStringLiteral("length")).toInt(), 2);

    const QJSValue firstEntry = jsArrayEntry(entries, 0);
    const QJSValue secondEntry = jsArrayEntry(entries, 1);
    QVERIFY(firstEntry.isObject());
    QVERIFY(secondEntry.isObject());
    QCOMPARE(firstEntry.property(QStringLiteral("contentY")).toInt(), 11);
    QCOMPARE(firstEntry.property(QStringLiteral("contentHeight")).toInt(), 18);
    QCOMPARE(firstEntry.property(QStringLiteral("contentAvailableWidth")).toInt(), 200);
    QCOMPARE(firstEntry.property(QStringLiteral("contentWidth")).toInt(), 35);
    QCOMPARE(firstEntry.property(QStringLiteral("visualRowWidths")).property(QStringLiteral("length")).toInt(), 1);
    QCOMPARE(jsArrayEntry(firstEntry.property(QStringLiteral("visualRowWidths")), 0).toInt(), 35);
    QCOMPARE(secondEntry.property(QStringLiteral("contentY")).toInt(), 29);
    QCOMPARE(secondEntry.property(QStringLiteral("contentHeight")).toInt(), 23);
    QCOMPARE(secondEntry.property(QStringLiteral("contentAvailableWidth")).toInt(), 200);
    QCOMPARE(secondEntry.property(QStringLiteral("contentWidth")).toInt(), 32);
    QCOMPARE(secondEntry.property(QStringLiteral("visualRowWidths")).property(QStringLiteral("length")).toInt(), 1);
    QCOMPARE(jsArrayEntry(secondEntry.property(QStringLiteral("visualRowWidths")), 0).toInt(), 32);
}

void WhatSonCppRegressionTests::logicalLineLayoutSupport_fallsBackWhenLiveEditorGeometryIsUnavailable()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/models/editor/structure/ContentsLogicalLineLayoutSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue buildEntries = library.property(QStringLiteral("buildEntries"));
    QVERIFY(buildEntries.isCallable());

    const QJSValue entries = buildEntries.call(QJSValueList {
        QJSValue(QStringLiteral("")),
        QJSValue(0),
        QJSValue(QJSValue::UndefinedValue),
        QJSValue(QJSValue::UndefinedValue),
        QJSValue(12),
    });

    QVERIFY2(!entries.isError(), qPrintable(entries.toString()));
    QCOMPARE(entries.property(QStringLiteral("length")).toInt(), 1);

    const QJSValue onlyEntry = jsArrayEntry(entries, 0);
    QVERIFY(onlyEntry.isObject());
    QCOMPARE(onlyEntry.property(QStringLiteral("contentY")).toInt(), 0);
    QCOMPARE(onlyEntry.property(QStringLiteral("contentHeight")).toInt(), 12);
    QCOMPARE(onlyEntry.property(QStringLiteral("contentWidth")).toInt(), 0);
    QCOMPARE(onlyEntry.property(QStringLiteral("visualRowWidths")).property(QStringLiteral("length")).toInt(), 1);
    QCOMPARE(jsArrayEntry(onlyEntry.property(QStringLiteral("visualRowWidths")), 0).toInt(), 0);
}
