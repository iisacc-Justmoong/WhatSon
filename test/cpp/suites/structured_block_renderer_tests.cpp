#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::structuredBlockRenderer_publishesSingleNormalizedInteractiveStream()
{
    ContentsStructuredBlockRenderer renderer;
    renderer.setSourceText(
        QStringLiteral("alpha\nbeta\n<resource type=\"image\" path=\"cover.png\" />\ngamma\ndelta"));

    const QVariantList renderedBlocks = renderer.renderedDocumentBlocks();
    QCOMPARE(renderedBlocks.size(), 3);

    const QVariantMap leadingGroup = renderedBlocks.at(0).toMap();
    QCOMPARE(leadingGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QVERIFY(leadingGroup.value(QStringLiteral("flattenedInteractiveGroup")).toBool());
    QCOMPARE(leadingGroup.value(QStringLiteral("sourceText")).toString(), QStringLiteral("alpha\nbeta"));
    QCOMPARE(leadingGroup.value(QStringLiteral("flattenedInteractiveChildCount")).toInt(), 2);

    const QVariantMap resourceBlock = renderedBlocks.at(1).toMap();
    QCOMPARE(resourceBlock.value(QStringLiteral("type")).toString(), QStringLiteral("resource"));

    const QVariantMap trailingGroup = renderedBlocks.at(2).toMap();
    QCOMPARE(trailingGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QCOMPARE(trailingGroup.value(QStringLiteral("sourceText")).toString(), QStringLiteral("gamma\ndelta"));
    QCOMPARE(trailingGroup.value(QStringLiteral("flattenedInteractiveChildCount")).toInt(), 2);

    QVERIFY(renderer.hasRenderedBlocks());
    QVERIFY(renderer.hasNonResourceRenderedBlocks());
}

void WhatSonCppRegressionTests::structuredBlockRenderer_keepsEmptyNotesFocusableWithOneTextGroup()
{
    ContentsStructuredBlockRenderer renderer;
    renderer.requestRenderRefresh();

    const QVariantList renderedBlocks = renderer.renderedDocumentBlocks();
    QCOMPARE(renderedBlocks.size(), 1);

    const QVariantMap emptyGroup = renderedBlocks.constFirst().toMap();
    QCOMPARE(emptyGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QVERIFY(emptyGroup.value(QStringLiteral("flattenedInteractiveGroup")).toBool());
    QCOMPARE(emptyGroup.value(QStringLiteral("sourceText")).toString(), QString());
    QCOMPARE(
        emptyGroup.value(QStringLiteral("flattenedInteractiveChildCount")).toInt(),
        emptyGroup.value(QStringLiteral("groupedBlocks")).toList().size());

    QVERIFY(renderer.hasRenderedBlocks());
    QVERIFY(renderer.hasNonResourceRenderedBlocks());
}

void WhatSonCppRegressionTests::structuredBlockRenderer_keepsTrailingResourceInsertionsEditable()
{
    ContentsStructuredBlockRenderer renderer;
    renderer.setSourceText(QStringLiteral("alpha\n<resource type=\"image\" path=\"cover.png\" />\n"));

    const QVariantList renderedBlocks = renderer.renderedDocumentBlocks();
    QCOMPARE(renderedBlocks.size(), 3);

    const QVariantMap resourceBlock = renderedBlocks.at(1).toMap();
    QCOMPARE(resourceBlock.value(QStringLiteral("type")).toString(), QStringLiteral("resource"));
    QVERIFY(resourceBlock.value(QStringLiteral("atomicBlock")).toBool());

    const QVariantMap trailingGroup = renderedBlocks.at(2).toMap();
    QCOMPARE(trailingGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QVERIFY(trailingGroup.value(QStringLiteral("textEditable")).toBool());
    QCOMPARE(trailingGroup.value(QStringLiteral("sourceText")).toString(), QString());
    QCOMPARE(
        trailingGroup.value(QStringLiteral("sourceStart")).toInt(),
        static_cast<int>(QStringLiteral("alpha\n<resource type=\"image\" path=\"cover.png\" />\n").size()));
}
