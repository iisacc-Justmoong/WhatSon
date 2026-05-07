#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::wysiwygEditorPolicy_mapsVisibleSelectionBackToRawSource()
{
    ContentsWysiwygEditorPolicy policy;
    const QString formattedSource = QStringLiteral("<b>Alpha</b>");

    QCOMPARE(policy.boundedOffset(99, formattedSource.size()), formattedSource.size());
    QCOMPARE(policy.normalizedSourceTagName(QStringLiteral("<  /b>")), QStringLiteral("b"));
    QVERIFY(policy.sourceOffsetIsInsideTagToken(formattedSource, 1));

    const QVariantMap tokenBounds = policy.sourceTagTokenBoundsForCursor(formattedSource, 1);
    QVERIFY(tokenBounds.value(QStringLiteral("inside")).toBool());
    QCOMPARE(tokenBounds.value(QStringLiteral("start")).toInt(), 0);
    QCOMPARE(tokenBounds.value(QStringLiteral("end")).toInt(), 2);

    QVERIFY(!policy.sourceRangeContainsVisibleLogicalContent(formattedSource, 0, 3));
    QVERIFY(policy.sourceRangeContainsVisibleLogicalContent(formattedSource, 3, 8));
    QVERIFY(policy.sourceRangeContainsVisibleLogicalContent(
        QStringLiteral("<resource type=\"image\" path=\"asset.png\" />"),
        0,
        42));

    ContentsEditorPresentationProjection projection;
    projection.setSourceText(formattedSource);
    const int visibleLength = projection.logicalText().size();
    QVERIFY(visibleLength > 0);

    const QVariantMap rawSelection = policy.rawSelectionForVisibleSurfaceSelection(
        &projection,
        1,
        4,
        4,
        visibleLength,
        formattedSource.size());
    QVERIFY(rawSelection.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        rawSelection.value(QStringLiteral("selectionStart")).toInt(),
        projection.sourceOffsetForVisibleLogicalOffset(1, visibleLength));
    QCOMPARE(
        rawSelection.value(QStringLiteral("selectionEnd")).toInt(),
        projection.sourceOffsetForVisibleLogicalOffset(4, visibleLength));
    QVERIFY(rawSelection.value(QStringLiteral("selectionStart")).toInt() > 0);
    const QVariantMap visibleContentSelection = policy.visibleContentSourceSelectionRange(
        QStringLiteral("<bold>Alpha</bold> beta"),
        0,
        QStringLiteral("<bold>Alpha").size());
    QCOMPARE(
        visibleContentSelection.value(QStringLiteral("start")).toInt(),
        QStringLiteral("<bold>").size());
    QCOMPARE(
        visibleContentSelection.value(QStringLiteral("end")).toInt(),
        QStringLiteral("<bold>Alpha").size());

    const QString boldSource = QStringLiteral("<bold>Alpha</bold>");
    projection.setSourceText(boldSource);
    const QVariantMap firstGlyphBackspacePayload = policy.visibleBackspaceMutationPayload(
        boldSource,
        &projection,
        1,
        projection.logicalText().size());
    QVERIFY(firstGlyphBackspacePayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        firstGlyphBackspacePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>lpha</bold>"));
    QCOMPARE(firstGlyphBackspacePayload.value(QStringLiteral("surfaceCursor")).toInt(), 0);

    const QString nestedStyledSource =
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>");
    projection.setSourceText(nestedStyledSource);
    const int nestedVisibleLength = projection.logicalText().size();
    QCOMPARE(projection.logicalText(), QStringLiteral("Alpha Beta"));
    const QVariantMap nestedBoundaryBackspacePayload = policy.visibleBackspaceMutationPayload(
        nestedStyledSource,
        &projection,
        QStringLiteral("Alpha").size(),
        nestedVisibleLength);
    QVERIFY(nestedBoundaryBackspacePayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        nestedBoundaryBackspacePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Al<italic>ph</italic></bold><italic> Beta</italic>"));
    QCOMPARE(
        nestedBoundaryBackspacePayload.value(QStringLiteral("deletedSourceStart")).toInt(),
        nestedStyledSource.indexOf(QStringLiteral("pha")) + 2);
    QCOMPARE(
        nestedBoundaryBackspacePayload.value(QStringLiteral("deletedSourceEnd")).toInt(),
        nestedStyledSource.indexOf(QStringLiteral("</italic>")));
    QCOMPARE(
        nestedBoundaryBackspacePayload.value(QStringLiteral("surfaceCursor")).toInt(),
        QStringLiteral("Alph").size());

    const QVariantMap visibleInsertionPayload = policy.visibleTextMutationPayload(
        nestedStyledSource,
        &projection,
        projection.logicalText(),
        QStringLiteral("AlphaX Beta"),
        QStringLiteral("AlphaX").size());
    QVERIFY(visibleInsertionPayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        visibleInsertionPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Al<italic>pha</italic></bold>X<italic> Beta</italic>"));
    QCOMPARE(
        visibleInsertionPayload.value(QStringLiteral("surfaceCursor")).toInt(),
        QStringLiteral("AlphaX").size());

    const QVariantMap visibleDeletePayload = policy.visibleTextMutationPayload(
        nestedStyledSource,
        &projection,
        projection.logicalText(),
        QStringLiteral("Alph Beta"),
        QStringLiteral("Alph").size());
    QVERIFY(visibleDeletePayload.value(QStringLiteral("applied")).toBool());
    QCOMPARE(
        visibleDeletePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<bold>Al<italic>ph</italic></bold><italic> Beta</italic>"));

    const QVariantMap resourceBlock{
        {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
        {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
        {QStringLiteral("htmlBlockIsDisplayBlock"), true},
        {QStringLiteral("sourceStart"), 10},
        {QStringLiteral("sourceEnd"), 20},
    };
    const QVariantMap invalidResourceBlockMissingStart{
        {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
        {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
        {QStringLiteral("htmlBlockIsDisplayBlock"), true},
        {QStringLiteral("sourceEnd"), 4},
    };
    const QVariantMap invalidResourceBlockMissingEnd{
        {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
        {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
        {QStringLiteral("htmlBlockIsDisplayBlock"), true},
        {QStringLiteral("sourceStart"), 2},
    };
    const QVariantList blocks{resourceBlock};
    const QVariantList invalidBlocks{invalidResourceBlockMissingStart, invalidResourceBlockMissingEnd};
    QVERIFY(policy.hasAtomicRenderedResourceBlocks(blocks));
    QVERIFY(policy.sourceRangeIntersectsAtomicResourceBlock(blocks, 12, 13));
    QVERIFY(!policy.sourceRangeIntersectsAtomicResourceBlock(blocks, 0, 5));
    QVERIFY(!policy.sourceRangeIntersectsAtomicResourceBlock(invalidBlocks, 0, 1));
    QCOMPARE(policy.htmlBlockSourceStart(invalidResourceBlockMissingStart), -1);
    QCOMPARE(policy.htmlBlockSourceEnd(invalidResourceBlockMissingStart), 4);
    QCOMPARE(policy.htmlBlockSourceStart(invalidResourceBlockMissingEnd), 2);
    QCOMPARE(policy.htmlBlockSourceEnd(invalidResourceBlockMissingEnd), -1);
    const QVariantMap invalidLogicalRange = policy.resourceLogicalRangeForBlock(
        invalidResourceBlockMissingStart,
        nullptr);
    QCOMPARE(invalidLogicalRange.value(QStringLiteral("start")).toInt(), 0);
    QCOMPARE(invalidLogicalRange.value(QStringLiteral("end")).toInt(), 0);

    const QVariantMap renderedSelection = policy.renderedLogicalSelectionRange(
        QStringLiteral("012345678901234567890123456789"),
        blocks,
        nullptr,
        12,
        13,
        30);
    QCOMPARE(renderedSelection.value(QStringLiteral("start")).toInt(), 10);
    QCOMPARE(renderedSelection.value(QStringLiteral("end")).toInt(), 20);
    const QVariantMap invalidRenderedSelection = policy.renderedLogicalSelectionRange(
        QStringLiteral("012345678901234567890123456789"),
        invalidBlocks,
        nullptr,
        0,
        1,
        30);
    QCOMPARE(invalidRenderedSelection.value(QStringLiteral("start")).toInt(), 0);
    QCOMPARE(invalidRenderedSelection.value(QStringLiteral("end")).toInt(), 1);

    const QVariantMap forwardPlan = policy.hiddenTagCursorNormalizationPlan(
        formattedSource,
        1,
        0,
        true,
        false,
        false,
        false);
    QVERIFY(forwardPlan.value(QStringLiteral("changed")).toBool());
    QCOMPARE(forwardPlan.value(QStringLiteral("targetCursorPosition")).toInt(), 3);
    QVERIFY(forwardPlan.value(QStringLiteral("clearVisiblePointerCursor")).toBool());

    const QVariantMap backwardPlan = policy.hiddenTagCursorNormalizationPlan(
        formattedSource,
        2,
        3,
        true,
        false,
        false,
        true);
    QVERIFY(backwardPlan.value(QStringLiteral("changed")).toBool());
    QCOMPARE(backwardPlan.value(QStringLiteral("targetCursorPosition")).toInt(), 0);
    QVERIFY(!backwardPlan.value(QStringLiteral("clearVisiblePointerCursor")).toBool());

    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" path=\"cover.png\" />");
    const QString resourceSource =
        QStringLiteral("Alpha\n") + resourceTag + QStringLiteral("\nBeta");
    const int resourceStart = QStringLiteral("Alpha\n").size();
    const int resourceEnd = resourceStart + resourceTag.size();
    ContentsEditorPresentationProjection resourceProjection;
    resourceProjection.setSourceText(resourceSource);
    const QVariantList resourceBlocks{
        QVariantMap{
            {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
            {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
            {QStringLiteral("htmlBlockIsDisplayBlock"), true},
            {QStringLiteral("sourceStart"), resourceStart},
            {QStringLiteral("sourceEnd"), resourceEnd},
        },
    };
    const int resourceLogicalStart = resourceProjection.logicalOffsetForSourceOffsetWithAffinity(resourceStart, false);
    const int resourceLogicalEnd = resourceProjection.logicalOffsetForSourceOffsetWithAffinity(resourceEnd, true);
    const QVariantMap resourceForwardPlan = policy.atomicResourceCursorNormalizationPlan(
        resourceSource,
        resourceBlocks,
        &resourceProjection,
        resourceLogicalStart,
        resourceStart - 1,
        resourceProjection.logicalText().size(),
        true,
        false,
        false);
    QVERIFY(resourceForwardPlan.value(QStringLiteral("resourceCursorActive")).toBool());
    QVERIFY(resourceForwardPlan.value(QStringLiteral("changed")).toBool());
    QCOMPARE(resourceForwardPlan.value(QStringLiteral("targetSourceCursor")).toInt(), resourceEnd + 1);
    QCOMPARE(resourceForwardPlan.value(QStringLiteral("targetLogicalCursor")).toInt(), resourceLogicalEnd + 1);

    const QVariantMap resourceBackwardPlan = policy.atomicResourceCursorNormalizationPlan(
        resourceSource,
        resourceBlocks,
        &resourceProjection,
        resourceLogicalEnd,
        resourceEnd + 1,
        resourceProjection.logicalText().size(),
        true,
        false,
        false);
    QVERIFY(resourceBackwardPlan.value(QStringLiteral("resourceCursorActive")).toBool());
    QVERIFY(resourceBackwardPlan.value(QStringLiteral("changed")).toBool());
    QCOMPARE(resourceBackwardPlan.value(QStringLiteral("targetSourceCursor")).toInt(), resourceStart - 1);
    QCOMPARE(resourceBackwardPlan.value(QStringLiteral("targetLogicalCursor")).toInt(), resourceLogicalStart - 1);

    ContentsEditorPresentationProjection resourceOnlyProjection;
    resourceOnlyProjection.setSourceText(resourceTag);
    const QVariantList resourceOnlyBlocks{
        QVariantMap{
            {QStringLiteral("renderDelegateType"), QStringLiteral("resource")},
            {QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock")},
            {QStringLiteral("htmlBlockIsDisplayBlock"), true},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), resourceTag.size()},
        },
    };
    const QVariantMap resourceOnlyPlan = policy.atomicResourceCursorNormalizationPlan(
        resourceTag,
        resourceOnlyBlocks,
        &resourceOnlyProjection,
        0,
        0,
        resourceOnlyProjection.logicalText().size(),
        true,
        false,
        false);
    QVERIFY(resourceOnlyPlan.value(QStringLiteral("resourceCursorActive")).toBool());
    QVERIFY(!resourceOnlyPlan.value(QStringLiteral("changed")).toBool());

    const QVariantMap lineRange = policy.visibleLogicalLineRange(
        QStringLiteral("one\ntwo\nthree"),
        5);
    QCOMPARE(lineRange.value(QStringLiteral("start")).toInt(), 4);
    QCOMPARE(lineRange.value(QStringLiteral("end")).toInt(), 7);

    const QVariantMap paragraphRange = policy.visibleLogicalParagraphRange(
        QStringLiteral("one\n\ntwo\n\nthree"),
        5);
    QCOMPARE(paragraphRange.value(QStringLiteral("start")).toInt(), 5);
    QCOMPARE(paragraphRange.value(QStringLiteral("end")).toInt(), 8);
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_delegatesWysiwygPolicyToCpp()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml"));
    const QString policyHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsWysiwygEditorPolicy.hpp"));
    const QString policySource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsWysiwygEditorPolicy.cpp"));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("ContentsWysiwygEditorPolicy {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("id: wysiwygEditorPolicy")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.sourceTagTokenBoundsForCursor(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.sourceRangeContainsVisibleLogicalContent(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.sourceRangeIntersectsAtomicResourceBlock(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.rawSelectionForVisibleSurfaceSelection(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.hiddenTagCursorNormalizationPlan(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("wysiwygEditorPolicy.atomicResourceCursorNormalizationPlan(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function normalizeCursorPositionAwayFromAtomicResourceBlock()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool atomicResourceCursorActive")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("/[A-Za-z0-9_.:-]/")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("lastIndexOf(\"<\"")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("indexOf(\">\"")));

    QVERIFY(policyHeader.contains(QStringLiteral("class ContentsWysiwygEditorPolicy")));
    QVERIFY(policyHeader.contains(QStringLiteral("Q_INVOKABLE QVariantMap rawSelectionForVisibleSurfaceSelection")));
    QVERIFY(policyHeader.contains(QStringLiteral("Q_INVOKABLE QVariantMap atomicResourceCursorNormalizationPlan")));
    QVERIFY(policyHeader.contains(QStringLiteral("Q_INVOKABLE QVariantMap visibleBackspaceMutationPayload")));
    QVERIFY(policySource.contains(QStringLiteral("sourceOffsetForVisibleLogicalOffset")));
    QVERIFY(policySource.contains(QStringLiteral("logicalOffsetForSourceOffsetWithAffinity")));
    QVERIFY(policySource.contains(QStringLiteral("sourceOffsetBeforeClosingInlineBoundaries")));
    QVERIFY(policySource.contains(QStringLiteral("htmlBlockObjectSource")));
}
