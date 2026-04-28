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

void WhatSonCppRegressionTests::structuredBlockRenderer_reportsAsyncRenderProfileForLargeStructuredDocuments()
{
    ContentsStructuredBlockRenderer renderer;
    renderer.setBackgroundRefreshEnabled(true);

    QString sourceText;
    sourceText.reserve(4096);
    sourceText += QStringLiteral("alpha\n");
    for (int index = 0; index < 48; ++index)
    {
        sourceText += QStringLiteral("<resource type=\"image\" path=\"cover-%1.png\" />\n")
                          .arg(index);
    }
    sourceText += QStringLiteral("omega");

    renderer.setSourceText(sourceText);

    QTRY_VERIFY_WITH_TIMEOUT(!renderer.renderPending(), 5000);

    const QVariantMap renderProfile = renderer.lastRenderProfile();
    QCOMPARE(renderProfile.value(QStringLiteral("mode")).toString(), QStringLiteral("async"));
    QCOMPARE(renderProfile.value(QStringLiteral("sourceLength")).toInt(), sourceText.size());
    QCOMPARE(renderProfile.value(QStringLiteral("blockCount")).toInt(), renderer.renderedDocumentBlocks().size());
    QVERIFY(renderProfile.value(QStringLiteral("elapsedMs")).toLongLong() >= 0);
    QVERIFY(renderProfile.value(QStringLiteral("blockCount")).toInt() >= 2);
    QVERIFY(renderer.hasRenderedBlocks());
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

void WhatSonCppRegressionTests::structuredBlockRenderer_keepsResourceOnlyTrailingLineEditable()
{
    const QString sourceText = QStringLiteral("<resource type=\"image\" path=\"cover.png\" />\n");
    ContentsStructuredBlockRenderer renderer;
    renderer.setSourceText(sourceText);

    const QVariantList renderedBlocks = renderer.renderedDocumentBlocks();
    QCOMPARE(renderedBlocks.size(), 2);

    const QVariantMap resourceBlock = renderedBlocks.at(0).toMap();
    QCOMPARE(resourceBlock.value(QStringLiteral("type")).toString(), QStringLiteral("resource"));
    QVERIFY(resourceBlock.value(QStringLiteral("atomicBlock")).toBool());

    const QVariantMap trailingGroup = renderedBlocks.at(1).toMap();
    QCOMPARE(trailingGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QVERIFY(trailingGroup.value(QStringLiteral("textEditable")).toBool());
    QVERIFY(!trailingGroup.value(QStringLiteral("atomicBlock")).toBool());
    QVERIFY(!trailingGroup.value(QStringLiteral("gutterCollapsed")).toBool());
    QCOMPARE(trailingGroup.value(QStringLiteral("logicalLineCountHint")).toInt(), 1);
    QCOMPARE(trailingGroup.value(QStringLiteral("sourceText")).toString(), QString());
    QCOMPARE(trailingGroup.value(QStringLiteral("sourceStart")).toInt(), static_cast<int>(sourceText.size()));
    QCOMPARE(trailingGroup.value(QStringLiteral("sourceEnd")).toInt(), static_cast<int>(sourceText.size()));
}

void WhatSonCppRegressionTests::structuredBlockRenderer_keepsEmptyParagraphBetweenResourcesEditable()
{
    const QString sourceText = QStringLiteral(
        "<resource type=\"image\" path=\"first.png\" />\n"
        "\n"
        "<resource type=\"image\" path=\"second.png\" />");
    ContentsStructuredBlockRenderer renderer;
    renderer.setSourceText(sourceText);

    const QVariantList renderedBlocks = renderer.renderedDocumentBlocks();
    QCOMPARE(renderedBlocks.size(), 3);

    QCOMPARE(renderedBlocks.at(0).toMap().value(QStringLiteral("type")).toString(), QStringLiteral("resource"));
    const QVariantMap emptyParagraphGroup = renderedBlocks.at(1).toMap();
    QCOMPARE(emptyParagraphGroup.value(QStringLiteral("type")).toString(), QStringLiteral("text-group"));
    QVERIFY(emptyParagraphGroup.value(QStringLiteral("textEditable")).toBool());
    QVERIFY(!emptyParagraphGroup.value(QStringLiteral("atomicBlock")).toBool());
    QVERIFY(!emptyParagraphGroup.value(QStringLiteral("gutterCollapsed")).toBool());
    QCOMPARE(emptyParagraphGroup.value(QStringLiteral("sourceText")).toString(), QString());
    QCOMPARE(renderedBlocks.at(2).toMap().value(QStringLiteral("type")).toString(), QStringLiteral("resource"));
}
