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
