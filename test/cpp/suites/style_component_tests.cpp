#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::styleComponent_ownsStyleRawTokenProjection()
{
    using WhatSon::EditorComponent::Style;

    QCOMPARE(Style::openingToken(), QStringLiteral("<style>"));
    QCOMPARE(Style::closingToken(), QStringLiteral("</style>"));
    QCOMPARE(
        Style::styleAttributeValues(),
        QStringList({
            QStringLiteral("Title"),
            QStringLiteral("Title2"),
            QStringLiteral("Subtitle"),
            QStringLiteral("Header"),
            QStringLiteral("Header2"),
            QStringLiteral("Body"),
            QStringLiteral("Description"),
            QStringLiteral("Caption"),
            QStringLiteral("Footnote")
        }));
    QCOMPARE(Style::normalizedStyleAttributeValue(QString()), QStringLiteral("Body"));
    QCOMPARE(Style::normalizedStyleAttributeValue(QStringLiteral("subtitle")), QStringLiteral("Subtitle"));
    QCOMPARE(Style::normalizedStyleAttributeValue(QStringLiteral("foot note")), QStringLiteral("Footnote"));
    QCOMPARE(Style::openingTokenForStyleAttributeValue(QStringLiteral("Title")), QStringLiteral("<style style=\"Title\">"));
    QCOMPARE(Style::openingTokenForStyleAttributeValue(QStringLiteral("Body")), QStringLiteral("<style>"));
    QCOMPARE(Style::openingTokenForStyleAttributeValue(QStringLiteral("Footnote")), QStringLiteral("<style style=\"Footnote\">"));

    const WhatSon::EditorComponent::StyleToken titleToken =
        Style::lvrsTextStyleTokenFromName(QStringLiteral("Title"));
    QVERIFY(titleToken.valid);
    QCOMPARE(titleToken.pixelSize, 26);
    QCOMPARE(titleToken.weight, QFont::Bold);
    QCOMPARE(titleToken.lineHeight, 26);
    QCOMPARE(titleToken.color, QStringLiteral("#E5FFFFFF"));

    const WhatSon::EditorComponent::StyleToken bodyFallbackToken =
        Style::lvrsTextStyleTokenFromName(QString());
    QVERIFY(bodyFallbackToken.valid);
    QCOMPARE(bodyFallbackToken.name, QStringLiteral("body"));

    const WhatSon::EditorComponent::StyleToken subtitleToken =
        Style::lvrsTextStyleTokenFromName(QStringLiteral("Subtitle"));
    QVERIFY(subtitleToken.valid);
    QCOMPARE(subtitleToken.pixelSize, 17);
    QCOMPARE(subtitleToken.weight, QFont::Medium);
    QCOMPARE(subtitleToken.lineHeight, 17);
    QCOMPARE(subtitleToken.color, QStringLiteral("#CCFFFFFF"));

    const WhatSon::EditorComponent::StyleToken footnoteToken =
        Style::lvrsTextStyleTokenFromName(QStringLiteral("Footnote"));
    QVERIFY(footnoteToken.valid);
    QCOMPARE(footnoteToken.pixelSize, 10);
    QCOMPARE(footnoteToken.weight, QFont::Normal);
    QCOMPARE(footnoteToken.lineHeight, 10);
    QCOMPARE(footnoteToken.color, QStringLiteral("#66FFFFFF"));

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
