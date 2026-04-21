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
