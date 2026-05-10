#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorSetProperty_setsDynamicAttributesWithInferredValueTypes()
{
    SetProperty input;
    QSignalSpy propertySetSpy(&input, &SetProperty::propertySet);

    QVariantMap result = input.setPropertyInSource(
        QStringLiteral("<resource />"),
        0,
        QStringLiteral("property1"),
        QVariant(QStringLiteral("string & value")));
    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<resource property1=\"string &amp; value\" />"));
    QCOMPARE(result.value(QStringLiteral("valueKind")).toString(), QStringLiteral("string"));

    result = input.setPropertyInSource(
        result.value(QStringLiteral("bodySourceText")).toString(),
        0,
        QStringLiteral("property2"),
        QVariant(4));
    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<resource property1=\"string &amp; value\" property2=4 />"));
    QCOMPARE(result.value(QStringLiteral("valueKind")).toString(), QStringLiteral("int"));

    result = input.setPropertyInSource(
        result.value(QStringLiteral("bodySourceText")).toString(),
        0,
        QStringLiteral("property3"),
        QVariant(true));
    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<resource property1=\"string &amp; value\" property2=4 property3=true />"));
    QCOMPARE(result.value(QStringLiteral("valueKind")).toString(), QStringLiteral("bool"));

    result = input.setPropertyInSource(
        result.value(QStringLiteral("bodySourceText")).toString(),
        0,
        QStringLiteral("property4"),
        QVariant(4.25));
    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<resource property1=\"string &amp; value\" property2=4 property3=true property4=4.25 />"));
    QCOMPARE(result.value(QStringLiteral("valueKind")).toString(), QStringLiteral("float"));
    QCOMPARE(propertySetSpy.count(), 4);
}

void WhatSonCppRegressionTests::editorSetProperty_updatesExistingAttributeAndRejectsInvalidNames()
{
    SetProperty input;

    const QVariantMap updatedResult = input.setPropertyInSource(
        QStringLiteral("<resource property2=4 />"),
        0,
        QStringLiteral("property2"),
        QVariant(8));
    QVERIFY(updatedResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        updatedResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<resource property2=8 />"));
    QCOMPARE(
        updatedResult.value(QStringLiteral("serializedAttribute")).toString(),
        QStringLiteral("property2=8"));

    const QVariantMap invalidNameResult = input.setPropertyInSource(
        updatedResult.value(QStringLiteral("bodySourceText")).toString(),
        0,
        QStringLiteral("bad name"),
        QVariant(true));
    QVERIFY(!invalidNameResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        invalidNameResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<resource property2=8 />"));
    QVERIFY(input.lastError().contains(QStringLiteral("Invalid property name")));

    const QVariantMap invalidCursorResult = input.setPropertyInSource(
        QStringLiteral("plain text"),
        0,
        QStringLiteral("property3"),
        QVariant(false));
    QVERIFY(!invalidCursorResult.value(QStringLiteral("valid")).toBool());
    QVERIFY(invalidCursorResult.value(QStringLiteral("errorMessage")).toString().contains(QStringLiteral("tag")));
}

void WhatSonCppRegressionTests::editorSetProperty_serializesResourceAttributeIntoWsnbodyDocument()
{
    SetProperty input;
    const QString bodyDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("<resource />"));

    const QVariantMap result = input.setPropertyInBodyDocument(
        QStringLiteral("note"),
        bodyDocument,
        0,
        QStringLiteral("property3"),
        QVariant(true));

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    const QString mutatedBodyDocument = result.value(QStringLiteral("bodyDocumentText")).toString();
    QVERIFY(mutatedBodyDocument.contains(QStringLiteral("    <resource property3=true />\n")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(mutatedBodyDocument),
        QStringLiteral("<resource property3=true />"));
}
