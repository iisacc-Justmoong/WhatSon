#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorSurfaceModeSupport_switchesToResourceEditorForResourceListModels()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/models/editor/display/ContentsEditorSurfaceModeSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue resourceEditorVisible = library.property(QStringLiteral("resourceEditorVisible"));
    const QJSValue currentResourceEntry = library.property(QStringLiteral("currentResourceEntry"));
    const QJSValue hasCurrentResourceEntry = library.property(QStringLiteral("hasCurrentResourceEntry"));
    QVERIFY(resourceEditorVisible.isCallable());
    QVERIFY(currentResourceEntry.isCallable());
    QVERIFY(hasCurrentResourceEntry.isCallable());

    QJSValue resourceEntry = engine.newObject();
    resourceEntry.setProperty(QStringLiteral("displayName"), QStringLiteral("Cover.PNG"));
    resourceEntry.setProperty(QStringLiteral("renderMode"), QStringLiteral("image"));

    QJSValue resourceListModel = engine.newObject();
    resourceListModel.setProperty(QStringLiteral("noteBacked"), false);
    resourceListModel.setProperty(QStringLiteral("currentResourceEntry"), resourceEntry);

    QJSValue noteListModel = engine.newObject();
    noteListModel.setProperty(QStringLiteral("noteBacked"), true);
    noteListModel.setProperty(QStringLiteral("currentResourceEntry"), resourceEntry);

    QVERIFY(resourceEditorVisible.call(QJSValueList{resourceListModel}).toBool());
    QVERIFY(!resourceEditorVisible.call(QJSValueList{noteListModel}).toBool());
    QVERIFY(!resourceEditorVisible.call(QJSValueList{QJSValue(QJSValue::UndefinedValue)}).toBool());

    const QJSValue resolvedResourceEntry = currentResourceEntry.call(QJSValueList{resourceListModel});
    QVERIFY(resolvedResourceEntry.isObject());
    QCOMPARE(
        resolvedResourceEntry.property(QStringLiteral("displayName")).toString(),
        QStringLiteral("Cover.PNG"));
    QVERIFY(hasCurrentResourceEntry.call(QJSValueList{resourceListModel}).toBool());

    const QJSValue emptyResourceEntry = currentResourceEntry.call(QJSValueList{noteListModel});
    QVERIFY(emptyResourceEntry.isObject());
    QVERIFY(!hasCurrentResourceEntry.call(QJSValueList{noteListModel}).toBool());
    QVERIFY(emptyResourceEntry.property(QStringLiteral("displayName")).isUndefined());
}
