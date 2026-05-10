#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorGetProperty_capturesTypedKeyValuePairsFromResourceTag()
{
    GetProperty reader;
    QSignalSpy changedSpy(&reader, &GetProperty::propertiesChanged);
    QSignalSpy capturedSpy(&reader, &GetProperty::propertiesCaptured);

    const QVariantMap result = reader.readPropertiesFromSource(
        QStringLiteral("<resource property1=\"string &amp; value\" property2=4 property3=true property4=4.25 />"),
        1);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("tagName")).toString(), QStringLiteral("resource"));
    QCOMPARE(result.value(QStringLiteral("propertyCount")).toInt(), 4);

    const QVariantMap properties = result.value(QStringLiteral("properties")).toMap();
    QCOMPARE(properties.value(QStringLiteral("property1")).toString(), QStringLiteral("string & value"));
    QCOMPARE(properties.value(QStringLiteral("property2")).toLongLong(), 4);
    QCOMPARE(properties.value(QStringLiteral("property3")).toBool(), true);
    QCOMPARE(properties.value(QStringLiteral("property4")).toDouble(), 4.25);

    const QVariantMap valueKinds = result.value(QStringLiteral("valueKinds")).toMap();
    QCOMPARE(valueKinds.value(QStringLiteral("property1")).toString(), QStringLiteral("string"));
    QCOMPARE(valueKinds.value(QStringLiteral("property2")).toString(), QStringLiteral("int"));
    QCOMPARE(valueKinds.value(QStringLiteral("property3")).toString(), QStringLiteral("bool"));
    QCOMPARE(valueKinds.value(QStringLiteral("property4")).toString(), QStringLiteral("float"));

    QCOMPARE(reader.tagName(), QStringLiteral("resource"));
    QCOMPARE(reader.propertyCount(), 4);
    QCOMPARE(reader.properties(), properties);
    QCOMPARE(reader.valueKinds(), valueKinds);
    QVERIFY(reader.containsProperty(QStringLiteral("property2")));
    QCOMPARE(reader.propertyValue(QStringLiteral("property3")).toBool(), true);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(capturedSpy.count(), 1);
}

void WhatSonCppRegressionTests::editorGetProperty_updatesStoredStateAndClearsWhenNoTag()
{
    GetProperty reader;
    QSignalSpy changedSpy(&reader, &GetProperty::propertiesChanged);

    QVariantMap result = reader.readPropertiesFromSource(
        QStringLiteral("<resource property1=\"asset\" property2=12 />"),
        1);
    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(reader.propertyCount(), 2);

    result = reader.readPropertiesFromSource(QStringLiteral("plain text"), 0);
    QVERIFY(!result.value(QStringLiteral("valid")).toBool());
    QVERIFY(reader.properties().isEmpty());
    QVERIFY(reader.valueKinds().isEmpty());
    QCOMPARE(reader.tagName(), QString());
    QVERIFY(reader.lastError().contains(QStringLiteral("tag")));
    QCOMPARE(changedSpy.count(), 2);
}

void WhatSonCppRegressionTests::editorGetProperty_readsBodyDocumentAttributesIntoAppStore()
{
    GetProperty reader;
    QSignalSpy capturedSpy(&reader, &GetProperty::propertiesCaptured);
    const QString bodyDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("<resource property1=\"asset\" property2=12 property3=false />"));

    const QVariantMap result = reader.readPropertiesFromBodyDocument(bodyDocument, 1);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("bodyDocumentText")).toString(), bodyDocument);
    QCOMPARE(reader.tagName(), QStringLiteral("resource"));
    QCOMPARE(reader.propertyCount(), 3);
    QCOMPARE(reader.propertyValue(QStringLiteral("property1")).toString(), QStringLiteral("asset"));
    QCOMPARE(reader.propertyValue(QStringLiteral("property2")).toLongLong(), 12);
    QCOMPARE(reader.propertyValue(QStringLiteral("property3")).toBool(), false);
    QCOMPARE(capturedSpy.count(), 1);
    QCOMPARE(capturedSpy.takeFirst().at(0).toMap().value(QStringLiteral("bodyDocumentText")).toString(), bodyDocument);
}
