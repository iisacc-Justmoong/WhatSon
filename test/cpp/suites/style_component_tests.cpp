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

    struct StyleTokenExpectation final
    {
        QString value;
        int pixelSize = 0;
        int weight = 0;
        QString styleName;
        int lineHeight = 0;
    };

    const QVector<StyleTokenExpectation> styleExpectations = {
        {QStringLiteral("Title"), 26, QFont::Bold, QStringLiteral("Bold"), 26},
        {QStringLiteral("Title2"), 22, QFont::DemiBold, QStringLiteral("SemiBold"), 22},
        {QStringLiteral("Subtitle"), 15, QFont::Medium, QStringLiteral("Medium"), 15},
        {QStringLiteral("Header"), 17, QFont::Bold, QStringLiteral("Bold"), 17},
        {QStringLiteral("Header2"), 15, QFont::DemiBold, QStringLiteral("SemiBold"), 15},
        {QStringLiteral("Body"), 12, QFont::Medium, QStringLiteral("Medium"), 12},
        {QStringLiteral("Description"), 12, QFont::Normal, QStringLiteral("Regular"), 12},
        {QStringLiteral("Caption"), 11, QFont::DemiBold, QStringLiteral("SemiBold"), 11},
        {QStringLiteral("Footnote"), 11, QFont::Normal, QStringLiteral("Regular"), 11}
    };

    for (const StyleTokenExpectation& expectation : styleExpectations)
    {
        const WhatSon::EditorComponent::StyleToken token =
            Style::lvrsTextStyleTokenFromName(expectation.value);
        QVERIFY(token.valid);
        QCOMPARE(token.pixelSize, expectation.pixelSize);
        QCOMPARE(token.weight, expectation.weight);
        QCOMPARE(token.styleName, expectation.styleName);
        QCOMPARE(token.lineHeight, expectation.lineHeight);

        const QString tokenCss = Style::cssDeclarationFromRawToken(
            QStringLiteral("<style style=\"%1\">").arg(expectation.value));
        QVERIFY(tokenCss.contains(QStringLiteral("font-size:%1px;").arg(expectation.pixelSize)));
        QVERIFY(tokenCss.contains(QStringLiteral("font-weight:%1;").arg(expectation.weight)));
        QVERIFY(tokenCss.contains(QStringLiteral("line-height:%1px;").arg(expectation.lineHeight)));
        QVERIFY(!tokenCss.contains(QStringLiteral("color:")));
    }

    const WhatSon::EditorComponent::StyleToken bodyFallbackToken =
        Style::lvrsTextStyleTokenFromName(QString());
    QVERIFY(bodyFallbackToken.valid);
    QCOMPARE(bodyFallbackToken.name, QStringLiteral("body"));

    const QString rawOpening = QStringLiteral(
        "<style style=\"Title\" font=\"Pretendard\" weight=\"600\" size=14 "
        "color=\"#F3F5F8\" background=\"#262728\" align=\"center\" height=1.25>");
    const QString cssDeclaration = Style::cssDeclarationFromRawToken(rawOpening);
    QVERIFY(cssDeclaration.contains(QStringLiteral("font-family:'Pretendard';")));
    QCOMPARE(
        Style::cssDeclarationFromRawToken(QStringLiteral("<style font=\"American Typewriter\">")),
        QStringLiteral("font-family:'American Typewriter';"));
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
        QStringLiteral("<span style=\"font-family:'Pretendard';font-size:14px;font-weight:600;line-height:1.25;color:#F3F5F8;background-color:#262728;text-align:center;\">"),
        rawOpening));
    QVERIFY(Style::spanMatchesOpeningToken(
        QStringLiteral("<span style=\"font-family:'American Typewriter';\">"),
        QStringLiteral("<style font=\"American Typewriter\">")));

    const WhatSon::EditorComponent::StyleSourceBaseline baseline =
        Style::sourceBaselineFromOpeningToken(rawOpening);
    QCOMPARE(baseline.weight, 600);
    QCOMPARE(baseline.background, QStringLiteral("#262728"));
}
