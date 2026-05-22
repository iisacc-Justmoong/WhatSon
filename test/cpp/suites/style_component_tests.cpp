#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::styleComponent_ownsStyleRawTokenProjection()
{
    using WhatSon::EditorComponent::Style;

    QCOMPARE(Style::openingToken(), QStringLiteral("<style>"));
    QCOMPARE(Style::closingToken(), QStringLiteral("</style>"));

    const WhatSon::EditorComponent::StyleToken titleToken =
        Style::lvrsTextStyleTokenFromName(QStringLiteral("Title"));
    QVERIFY(titleToken.valid);
    QCOMPARE(titleToken.pixelSize, 26);
    QCOMPARE(titleToken.weight, QFont::Bold);
    QCOMPARE(titleToken.lineHeight, 26);
    QCOMPARE(titleToken.color, QStringLiteral("#E5FFFFFF"));

    const QString rawOpening = QStringLiteral(
        "<style style=\"Title\" font=\"Pretendard\" weight=\"600\" size=14 "
        "color=\"#F3F5F8\" background=\"#262728\" align=\"center\" height=1.25>");
    const QString cssDeclaration = Style::cssDeclarationFromRawToken(rawOpening);
    QVERIFY(cssDeclaration.contains(QStringLiteral("font-family:Pretendard;")));
    QVERIFY(cssDeclaration.contains(QStringLiteral("font-size:14px;")));
    QVERIFY(cssDeclaration.contains(QStringLiteral("font-weight:600;")));
    QVERIFY(cssDeclaration.contains(QStringLiteral("color:#F3F5F8;")));
    QVERIFY(cssDeclaration.contains(QStringLiteral("background-color:#262728;")));
    QVERIFY(cssDeclaration.contains(QStringLiteral("text-align:center;")));
    QVERIFY(cssDeclaration.contains(QStringLiteral("line-height:1.25;")));

    const QString openingHtml = Style::openingHtmlFromRawToken(rawOpening);
    QVERIFY(openingHtml.contains(QStringLiteral("<!--whatson-style-source:")));
    QVERIFY(openingHtml.contains(QStringLiteral("<span style=\"")));
    QCOMPARE(Style::closingHtml(), QStringLiteral("</span><a name=\"whatson-style-source-end\"></a><!--/whatson-style-source-->"));

    QVERIFY(Style::spanMatchesOpeningToken(
        QStringLiteral("<span style=\"font-family:Pretendard;font-size:14px;font-weight:600;line-height:1.25;color:#F3F5F8;background-color:#262728;text-align:center;\">"),
        rawOpening));

    const WhatSon::EditorComponent::StyleSourceBaseline baseline =
        Style::sourceBaselineFromOpeningToken(rawOpening);
    QCOMPARE(baseline.weight, 600);
    QCOMPARE(baseline.background, QStringLiteral("#262728"));
}
