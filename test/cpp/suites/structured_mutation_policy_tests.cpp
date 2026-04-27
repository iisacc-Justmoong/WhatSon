#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::structuredMutationPolicy_buildsDeletionAndInsertionPayloads()
{
    ContentsStructuredDocumentMutationPolicy policy;

    const QString sourceWithEmptyBlock = QStringLiteral("aa\n\nbb");
    const QVariantMap emptyBlockData{{QStringLiteral("sourceStart"), 3}};

    const QVariantMap backwardDeletion =
        policy.emptyTextBlockDeletionRange(emptyBlockData, QStringLiteral("backward"), sourceWithEmptyBlock);
    QCOMPARE(backwardDeletion.value(QStringLiteral("start")).toInt(), 2);
    QCOMPARE(backwardDeletion.value(QStringLiteral("end")).toInt(), 3);
    QCOMPARE(
        backwardDeletion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        2);

    const QVariantMap forwardDeletion =
        policy.emptyTextBlockDeletionRange(emptyBlockData, QStringLiteral("forward"), sourceWithEmptyBlock);
    QCOMPARE(forwardDeletion.value(QStringLiteral("start")).toInt(), 3);
    QCOMPARE(forwardDeletion.value(QStringLiteral("end")).toInt(), 4);
    QCOMPARE(
        forwardDeletion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        3);

    QCOMPARE(policy.nextEditableSourceOffsetAfterBlock(QStringLiteral("a\nb"), 1), 2);
    QCOMPARE(policy.nextEditableSourceOffsetAfterBlock(QStringLiteral("abc"), 2), 2);

    const QVariantMap structuredInsertion = policy.buildStructuredInsertionPayload(
        QStringLiteral("abc"),
        1,
        QStringLiteral("block"),
        2);
    QCOMPARE(
        structuredInsertion.value(QStringLiteral("insertedSourceText")).toString(),
        QStringLiteral("\nblock\n"));
    QCOMPARE(
        structuredInsertion.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("a\nblock\nbc"));
    QCOMPARE(structuredInsertion.value(QStringLiteral("sourceOffset")).toInt(), 4);

    const QString expectedResourceBlock =
        QStringLiteral("<resource one />\n<resource two />");
    const QString expectedInsertedResourceText =
        QStringLiteral("\n%1\n").arg(expectedResourceBlock);
    const QVariantMap resourceInsertion = policy.buildResourceInsertionPayload(
        QStringLiteral("abc"),
        1,
        QStringList{
            QStringLiteral("  <resource one />  "),
            QString(),
            QStringLiteral("<resource two />"),
        });
    QCOMPARE(
        resourceInsertion.value(QStringLiteral("insertedSourceText")).toString(),
        expectedInsertedResourceText);
    QCOMPARE(
        resourceInsertion.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("a%1bc").arg(expectedInsertedResourceText));

    const int expectedFocusOffset = 1 + 1 + static_cast<int>(expectedResourceBlock.size()) + 1;
    QCOMPARE(
        resourceInsertion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        expectedFocusOffset);
    QVERIFY(resourceInsertion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("preferNearestTextBlock")).toBool());

    const QVariantMap eofResourceInsertion = policy.buildResourceInsertionPayload(
        QStringLiteral("abc"),
        3,
        QStringList{QStringLiteral("<resource one />")});
    QCOMPARE(
        eofResourceInsertion.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("abc\n<resource one />\n"));
    QCOMPARE(
        eofResourceInsertion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        static_cast<int>(QStringLiteral("abc\n<resource one />\n").size()));
    QVERIFY(eofResourceInsertion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("preferNearestTextBlock")).toBool());

    QVERIFY(policy.buildResourceInsertionPayload(QString(), 0, QVariantList{}).isEmpty());
}

void WhatSonCppRegressionTests::structuredMutationPolicy_buildsParagraphBoundaryMergeAndSplitPayloads()
{
    ContentsStructuredDocumentMutationPolicy policy;

    const QVariantMap implicitParagraphBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("sourceStart"), 0},
        {QStringLiteral("sourceEnd"), 6},
    };
    const QVariantMap titleBlock{
        {QStringLiteral("type"), QStringLiteral("title")},
        {QStringLiteral("semanticTagName"), QStringLiteral("title")},
        {QStringLiteral("sourceStart"), 0},
        {QStringLiteral("sourceEnd"), 5},
    };
    QVERIFY(policy.supportsParagraphBoundaryOperations(implicitParagraphBlock));
    QVERIFY(!policy.supportsParagraphBoundaryOperations(titleBlock));

    const QString implicitMergeSource = QStringLiteral("foo\nbar");
    const QVariantMap implicitPreviousBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("sourceStart"), 0},
        {QStringLiteral("sourceEnd"), 3},
    };
    const QVariantMap implicitCurrentBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("sourceStart"), 4},
        {QStringLiteral("sourceEnd"), 7},
    };
    const QVariantMap implicitMergePayload = policy.buildParagraphMergePayload(
        implicitPreviousBlock,
        implicitCurrentBlock,
        implicitMergeSource);
    QCOMPARE(
        implicitMergePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("foobar"));
    QCOMPARE(
        implicitMergePayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        3);

    const QString explicitMergeSource =
        QStringLiteral("<paragraph>foo</paragraph>\n<paragraph>bar</paragraph>");
    const QVariantMap explicitPreviousBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("tagName"), QStringLiteral("paragraph")},
        {QStringLiteral("blockSourceStart"), 0},
        {QStringLiteral("blockSourceEnd"), 26},
        {QStringLiteral("contentStart"), 11},
        {QStringLiteral("contentEnd"), 14},
        {QStringLiteral("sourceStart"), 11},
        {QStringLiteral("sourceEnd"), 14},
    };
    const QVariantMap explicitCurrentBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("tagName"), QStringLiteral("paragraph")},
        {QStringLiteral("blockSourceStart"), 27},
        {QStringLiteral("blockSourceEnd"), 53},
        {QStringLiteral("contentStart"), 38},
        {QStringLiteral("contentEnd"), 41},
        {QStringLiteral("sourceStart"), 38},
        {QStringLiteral("sourceEnd"), 41},
    };
    const QVariantMap explicitMergePayload = policy.buildParagraphMergePayload(
        explicitPreviousBlock,
        explicitCurrentBlock,
        explicitMergeSource);
    QCOMPARE(
        explicitMergePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<paragraph>foobar</paragraph>"));
    QCOMPARE(
        explicitMergePayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        14);

    const QVariantMap implicitSplitPayload = policy.buildParagraphSplitPayload(
        implicitParagraphBlock,
        QStringLiteral("foobar"),
        3);
    QCOMPARE(
        implicitSplitPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("foo\nbar"));
    QCOMPARE(
        implicitSplitPayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        4);

    const QVariantMap explicitSplitPayload = policy.buildParagraphSplitPayload(
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("paragraph")},
            {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
            {QStringLiteral("tagName"), QStringLiteral("paragraph")},
            {QStringLiteral("blockSourceStart"), 0},
            {QStringLiteral("blockSourceEnd"), 29},
            {QStringLiteral("contentStart"), 11},
            {QStringLiteral("contentEnd"), 17},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 17},
        },
        QStringLiteral("<paragraph>foobar</paragraph>"),
        14);
    QCOMPARE(
        explicitSplitPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<paragraph>foo</paragraph>\n<paragraph>bar</paragraph>"));
    QCOMPARE(
        explicitSplitPayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        38);
}
