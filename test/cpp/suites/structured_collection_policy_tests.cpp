#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::structuredCollectionPolicy_normalizesEntriesAndPrefersResolvedMatches()
{
    ContentsStructuredDocumentCollectionPolicy policy;

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
