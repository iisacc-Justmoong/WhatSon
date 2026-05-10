#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorSetTag_insertsStaticCalloutPairIntoSourceSelection()
{
    SetTag input;
    QSignalSpy insertedSpy(&input, &SetTag::tagInserted);

    QCOMPARE(input.tagName(), QStringLiteral("callout"));

    const QVariantMap result = input.insertIntoSource(
        QStringLiteral("Alpha Beta"),
        6,
        4);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <callout>Beta</callout>"));
    QCOMPARE(
        result.value(QStringLiteral("cursorPosition")).toInt(),
        QStringLiteral("Alpha <callout>Beta</callout>").size());
    QCOMPARE(result.value(QStringLiteral("selectedText")).toString(), QStringLiteral("Beta"));
    QCOMPARE(insertedSpy.count(), 1);
}

void WhatSonCppRegressionTests::editorSetTag_usesStaticAgendaTemplateAndRejectsUnsupportedNames()
{
    SetTag input;
    QVERIFY(input.availableTagNames().contains(QStringLiteral("callout")));
    QVERIFY(input.availableTagNames().contains(QStringLiteral("agenda")));

    QVERIFY(input.configureTagName(QStringLiteral("agenda")));
    QCOMPARE(input.tagName(), QStringLiteral("agenda"));

    const QVariantMap agendaResult = input.insertIntoSource(
        QStringLiteral("Plan: "),
        6,
        0);
    QVERIFY(agendaResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        agendaResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Plan: <agenda><task></task></agenda>"));
    QCOMPARE(
        agendaResult.value(QStringLiteral("cursorPosition")).toInt(),
        QStringLiteral("Plan: <agenda><task>").size());

    QVERIFY(!input.configureTagName(QStringLiteral("script")));
    QCOMPARE(input.tagName(), QStringLiteral("agenda"));
    QVERIFY(input.lastError().contains(QStringLiteral("Unsupported static body tag")));

    const QVariantMap unsupportedResult = input.insertNamedTagIntoSource(
        QStringLiteral("<script>"),
        QStringLiteral("Alpha"),
        0,
        0);
    QVERIFY(!unsupportedResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        unsupportedResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha"));
}

void WhatSonCppRegressionTests::editorSetTag_serializesInsertedStaticTagIntoWsnbodyDocument()
{
    SetTag input;
    QSignalSpy insertedSpy(&input, &SetTag::tagInserted);
    const QString bodyDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("Alpha\nBeta"));

    const QVariantMap result = input.insertNamedTagIntoBodyDocument(
        QStringLiteral("callout"),
        QStringLiteral("note"),
        bodyDocument,
        6,
        4);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());

    const QString mutatedBodyDocument = result.value(QStringLiteral("bodyDocumentText")).toString();
    QCOMPARE(insertedSpy.count(), 1);
    const QVariantMap emittedResult = insertedSpy.takeFirst().at(0).toMap();
    QVERIFY(emittedResult.value(QStringLiteral("bodyDocumentText")).toString().contains(
        QStringLiteral("<callout>Beta</callout>")));
    QVERIFY(mutatedBodyDocument.contains(
        QStringLiteral("    <paragraph><callout>Beta</callout></paragraph>\n")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(mutatedBodyDocument),
        QStringLiteral("Alpha\n<callout>Beta</callout>"));
}
