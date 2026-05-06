#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::logicalTextBridge_advancesCursorPastClosingWebLinkTag()
{
    ContentsLogicalTextBridge bridge;
    const QString sourceText =
        QStringLiteral("<weblink href=\"www.iisacc.com\">site</weblink>!");
    bridge.setText(sourceText);

    QCOMPARE(bridge.logicalText(), QStringLiteral("site!"));
    QCOMPARE(bridge.logicalLengthForSourceText(sourceText), QStringLiteral("site!").size());
    QCOMPARE(bridge.sourceOffsetForLogicalOffset(QStringLiteral("site").size()), sourceText.indexOf(QLatin1Char('!')));
}

void WhatSonCppRegressionTests::logicalTextBridge_mapsSourceCursorInsideInlineTagsToVisibleBoundary()
{
    ContentsLogicalTextBridge bridge;
    const QString sourceText = QStringLiteral("<bold>Alpha</bold> beta");
    bridge.setText(sourceText);

    QCOMPARE(bridge.logicalText(), QStringLiteral("Alpha beta"));

    const int openingTagEnd = QStringLiteral("<bold>").size();
    const int closingTagStart = sourceText.indexOf(QStringLiteral("</bold>"));
    const int closingTagEnd = closingTagStart + QStringLiteral("</bold>").size();

    QCOMPARE(bridge.logicalOffsetForSourceOffset(0), 0);
    QCOMPARE(bridge.logicalOffsetForSourceOffset(1), 0);
    QCOMPARE(bridge.logicalOffsetForSourceOffset(openingTagEnd), 0);
    QCOMPARE(bridge.logicalOffsetForSourceOffset(openingTagEnd + 1), 1);
    QCOMPARE(bridge.logicalOffsetForSourceOffset(closingTagStart), QStringLiteral("Alpha").size());
    QCOMPARE(bridge.logicalOffsetForSourceOffset(closingTagStart + 1), QStringLiteral("Alpha").size());
    QCOMPARE(bridge.logicalOffsetForSourceOffset(closingTagEnd), QStringLiteral("Alpha").size());
    QCOMPARE(bridge.logicalOffsetForSourceOffset(closingTagEnd + 1), QStringLiteral("Alpha ").size());

    ContentsEditorPresentationProjection projection;
    projection.setSourceText(sourceText);
    QCOMPARE(projection.logicalOffsetForSourceOffset(closingTagStart + 1), QStringLiteral("Alpha").size());
}

void WhatSonCppRegressionTests::logicalTextBridge_notifiesLogicalToSourceOffsetChanges()
{
    ContentsLogicalTextBridge bridge;
    QSignalSpy bridgeOffsetsSpy(&bridge, &ContentsLogicalTextBridge::logicalToSourceOffsetsChanged);

    const QString styledSourceText = QStringLiteral("<bold>Alpha</bold>");
    bridge.setText(styledSourceText);
    QCOMPARE(bridgeOffsetsSpy.count(), 1);
    QCOMPARE(bridge.logicalText(), QStringLiteral("Alpha"));
    QCOMPARE(bridge.logicalToSourceOffsets().size(), bridge.logicalText().size() + 1);
    QCOMPARE(bridge.sourceOffsetForLogicalOffset(bridge.logicalText().size()), styledSourceText.size());
    QCOMPARE(bridge.sourceOffsetForVisibleLogicalOffset(bridge.logicalText().size() + 99, bridge.logicalText().size()), styledSourceText.size());
    QCOMPARE(bridge.logicalOffsetForSourceOffsetWithAffinity(styledSourceText.size(), true), bridge.logicalText().size());

    bridge.setText(styledSourceText);
    QCOMPARE(bridgeOffsetsSpy.count(), 1);

    const QString plainSourceText = QStringLiteral("Alpha");
    bridge.setText(plainSourceText);
    QCOMPARE(bridgeOffsetsSpy.count(), 2);
    QCOMPARE(bridge.logicalToSourceOffsets().size(), bridge.logicalText().size() + 1);
    QCOMPARE(bridge.sourceOffsetForLogicalOffset(bridge.logicalText().size()), plainSourceText.size());

    ContentsEditorPresentationProjection projection;
    QSignalSpy projectionCursorSpy(
        &projection,
        &ContentsEditorPresentationProjection::logicalCursorPositionChanged);
    projection.setSourceCursorPosition(styledSourceText.size());
    projection.setSourceText(styledSourceText);
    QVERIFY(projectionCursorSpy.count() >= 1);
    QCOMPARE(projection.logicalCursorPosition(), projection.logicalText().size());
    QCOMPARE(projection.sourceOffsetForVisibleLogicalOffset(projection.logicalText().size(), projection.logicalText().size()), styledSourceText.size());
    QCOMPARE(projection.logicalOffsetForSourceOffsetWithAffinity(styledSourceText.size(), true), projection.logicalText().size());
}
