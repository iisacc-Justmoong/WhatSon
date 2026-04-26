#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::structuredCollectionPolicy_normalizesEntriesAndPrefersResolvedMatches()
{
    ContentsStructuredDocumentCollectionPolicy policy;
    const QString structuredPolicySource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.cpp"));
    const QString oldStructuredPolicySource = readUtf8SourceFile(
        QStringLiteral("src/app/models/content/structured/ContentsStructuredDocumentCollectionPolicy.cpp"));
    QVERIFY(!structuredPolicySource.isEmpty());
    QVERIFY(oldStructuredPolicySource.isEmpty());

    const QVariantMap indexedEntries{
        {QStringLiteral("10"), QStringLiteral("ten")},
        {QStringLiteral("2"), QStringLiteral("two")},
        {QStringLiteral("0"), QStringLiteral("zero")},
        {QStringLiteral("ignored"), QStringLiteral("skip")},
    };
    const QVariantList normalizedEntries = policy.normalizeEntries(indexedEntries);
    QCOMPARE(normalizedEntries.size(), 3);
    QCOMPARE(normalizedEntries.at(0).toString(), QStringLiteral("zero"));
    QCOMPARE(normalizedEntries.at(1).toString(), QStringLiteral("two"));
    QCOMPARE(normalizedEntries.at(2).toString(), QStringLiteral("ten"));

    QCOMPARE(
        policy.normalizeSourceText(QStringLiteral("a\r\nb\rc\u2028d\u2029e\u00a0f")),
        QStringLiteral("a\nb\nc\nd\ne f"));
    QCOMPARE(policy.spliceSourceRange(QStringLiteral("abc"), -1, 99, QStringLiteral("X")), QStringLiteral("X"));
    QCOMPARE(policy.floorNumberOrFallback(QVariant(12.9), -1), 12);
    QCOMPARE(policy.floorNumberOrFallback(QVariant(QStringLiteral("not-a-number")), 7), 7);

    const QVariantMap blockEntry{
        {QStringLiteral("resourceIndex"), 7},
        {QStringLiteral("sourceStart"), 10},
        {QStringLiteral("sourceEnd"), 20},
        {QStringLiteral("resourceId"), QStringLiteral("resource-7")},
        {QStringLiteral("resourcePath"), QStringLiteral("/tmp/resource-7")},
    };
    const QVariantList renderedResources{
        QVariantMap{
            {QStringLiteral("index"), 7},
            {QStringLiteral("resourceId"), QStringLiteral("resource-7")},
        },
        QVariantMap{
            {QStringLiteral("index"), 7},
            {QStringLiteral("resourceId"), QStringLiteral("resource-7")},
            {QStringLiteral("resolvedPath"), QStringLiteral("/tmp/resource-7")},
        },
    };

    const QVariantMap resolvedEntry = policy.resourceEntryForBlock(blockEntry, renderedResources);
    QCOMPARE(resolvedEntry.value(QStringLiteral("index")).toInt(), 7);
    QCOMPARE(
        resolvedEntry.value(QStringLiteral("resolvedPath")).toString(),
        QStringLiteral("/tmp/resource-7"));
}

void WhatSonCppRegressionTests::structuredCollectionPolicy_normalizesQmlJsArrayEntries()
{
    ContentsStructuredDocumentCollectionPolicy policy;
    QJSEngine engine;

    const QJSValue jsArray = engine.evaluate(
        QStringLiteral("["
                       "  { type: 'text', sourceStart: 0, sourceEnd: 5, sourceText: 'alpha' },"
                       "  { type: 'break', sourceStart: 5, sourceEnd: 13, sourceText: '</break>' }"
                       "]"));
    QVERIFY2(!jsArray.isError(), "Failed to build QJSValue array fixture.");
    QVERIFY(jsArray.isArray());

    const QVariantList normalizedEntries = policy.normalizeEntries(QVariant::fromValue(jsArray));
    QCOMPARE(normalizedEntries.size(), 2);

    const QVariantMap firstBlock = normalizedEntries.at(0).toMap();
    QCOMPARE(firstBlock.value(QStringLiteral("type")).toString(), QStringLiteral("text"));
    QCOMPARE(firstBlock.value(QStringLiteral("sourceText")).toString(), QStringLiteral("alpha"));

    const QVariantMap secondBlock = normalizedEntries.at(1).toMap();
    QCOMPARE(secondBlock.value(QStringLiteral("type")).toString(), QStringLiteral("break"));
    QCOMPARE(secondBlock.value(QStringLiteral("sourceText")).toString(), QStringLiteral("</break>"));
}

void WhatSonCppRegressionTests::structuredCollectionPolicy_flattensImplicitInteractiveTextBlocksIntoSingleGroups()
{
    ContentsStructuredDocumentCollectionPolicy policy;

    const QString sourceText =
        QStringLiteral("alpha\nbeta\n<resource type=\"image\" path=\"cover.png\" />\ngamma\ndelta");
    const QVariantList renderedBlocks{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("paragraph")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
            {QStringLiteral("textEditable"), true},
            {QStringLiteral("atomicBlock"), false},
            {QStringLiteral("implicitTextBlock"), true},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("paragraph")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
            {QStringLiteral("textEditable"), true},
            {QStringLiteral("atomicBlock"), false},
            {QStringLiteral("implicitTextBlock"), true},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("resource")},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 53},
            {QStringLiteral("sourceText"), QStringLiteral("<resource type=\"image\" path=\"cover.png\" />")},
            {QStringLiteral("textEditable"), false},
            {QStringLiteral("atomicBlock"), true},
            {QStringLiteral("explicitBlock"), true},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("paragraph")},
            {QStringLiteral("sourceStart"), 54},
            {QStringLiteral("sourceEnd"), 59},
            {QStringLiteral("sourceText"), QStringLiteral("gamma")},
            {QStringLiteral("textEditable"), true},
            {QStringLiteral("atomicBlock"), false},
            {QStringLiteral("implicitTextBlock"), true},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("paragraph")},
            {QStringLiteral("sourceStart"), 60},
            {QStringLiteral("sourceEnd"), 65},
            {QStringLiteral("sourceText"), QStringLiteral("delta")},
            {QStringLiteral("textEditable"), true},
            {QStringLiteral("atomicBlock"), false},
            {QStringLiteral("implicitTextBlock"), true},
        },
    };

    const QVariantList normalizedBlocks =
        policy.normalizeInteractiveDocumentBlocks(renderedBlocks, sourceText);
    QCOMPARE(normalizedBlocks.size(), 3);

    const QVariantMap firstGroup = normalizedBlocks.at(0).toMap();
    QCOMPARE(firstGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QVERIFY(firstGroup.value(QStringLiteral("flattenedInteractiveGroup")).toBool());
    QCOMPARE(firstGroup.value(QStringLiteral("flattenedInteractiveChildCount")).toInt(), 2);
    QCOMPARE(firstGroup.value(QStringLiteral("sourceText")).toString(), QStringLiteral("alpha\nbeta"));
    QCOMPARE(firstGroup.value(QStringLiteral("logicalLineCountHint")).toInt(), 2);

    const QVariantMap resourceBlock = normalizedBlocks.at(1).toMap();
    QCOMPARE(resourceBlock.value(QStringLiteral("type")).toString(), QStringLiteral("resource"));

    const QVariantMap trailingGroup = normalizedBlocks.at(2).toMap();
    QCOMPARE(trailingGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QCOMPARE(trailingGroup.value(QStringLiteral("sourceText")).toString(), QStringLiteral("gamma\ndelta"));
    QCOMPARE(trailingGroup.value(QStringLiteral("flattenedInteractiveChildCount")).toInt(), 2);
}
