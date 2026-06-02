#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <QVector>

void WhatSonCppRegressionTests::noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks()
{
    const QString sourceText =
        QStringLiteral("브랜드 사이트 <weblink href=\"www.iisacc.com\">아이작닷컴</weblink>");
    const QString bodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);

    QVERIFY(bodyDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">아이작닷컴</weblink>")));
    QCOMPARE(WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument), sourceText);

    const QString htmlProjection =
        WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(bodyDocument);
    QVERIFY(htmlProjection.contains(
        QStringLiteral("<a href=\"https://www.iisacc.com\" style=\"color:#8CB4FF;text-decoration: underline;\">")));
    QVERIFY(htmlProjection.contains(QStringLiteral("아이작닷컴</a>")));

    const QString inlineStyleDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"));
    const QString inlineStyleHtmlProjection =
        WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(inlineStyleDocument);
    QCOMPARE(
        WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(inlineStyleDocument),
        QStringLiteral("Alpha Beta"));
    QVERIFY(inlineStyleHtmlProjection.contains(QStringLiteral("<strong style=\"font-weight:900;\">Al")));
    QVERIFY(inlineStyleHtmlProjection.contains(QStringLiteral("<span style=\"font-style:italic;\">pha</span>")));
    QVERIFY(inlineStyleHtmlProjection.contains(QStringLiteral("</strong><span style=\"font-style:italic;\"> Beta</span>")));
    QVERIFY(!inlineStyleHtmlProjection.contains(QStringLiteral("&lt;bold&gt;")));

    const QString autoWrappedDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("Visit www.iisacc.com"));
    QVERIFY(autoWrappedDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink>")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_projectsSourceToEditorHtmlWithExplicitBreaks()
{
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        QStringLiteral("First line\nSecond line"));

    QVERIFY(editorHtml.contains(QStringLiteral("font-family:Pretendard;")));
    QVERIFY(editorHtml.contains(QStringLiteral("font-size:12px;")));
    QVERIFY(editorHtml.contains(QStringLiteral("font-weight:500;")));
    QVERIFY(editorHtml.contains(QStringLiteral("line-height:12px;")));
    QVERIFY(editorHtml.contains(QStringLiteral("color:#CCFFFFFF;")));
    QVERIFY(editorHtml.contains(QStringLiteral("First line<br/>Second line")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), editorHtml),
        QStringLiteral("First line\nSecond line"));

    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    const QString roundTrippedEditorHtml = editorDocument.toHtml();
    QVERIFY(roundTrippedEditorHtml.contains(QStringLiteral("Pretendard")));
    QVERIFY(roundTrippedEditorHtml.contains(QStringLiteral("font-family:'Pretendard'")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), roundTrippedEditorHtml),
        QStringLiteral("First line\nSecond line"));

    const QString boldEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        QStringLiteral("Alpha <bold>Beta</bold> Gamma"));
    QTextDocument boldEditorDocument;
    boldEditorDocument.setHtml(boldEditorHtml);
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            boldEditorDocument.toHtml()),
        QStringLiteral("Alpha <style weight=\"900\">Beta</style> Gamma"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversEditorHtmlBreaksAsCanonicalSourceLines()
{
    const QString editorHtml = QStringLiteral(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /></head>"
        "<body><p>First line</p><p>Second line</p></body></html>");

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            editorHtml),
        QStringLiteral("First line\nSecond line"));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            QStringLiteral("First line<br/>Second line")),
        QStringLiteral("First line\nSecond line"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversEditorFormattingTagsFromRichText()
{
    const QString editorHtml = QStringLiteral(
        "Alpha <strong style=\"font-weight:900;\">Beta</strong> "
        "<span style=\"font-style:italic;\">Gamma</span> "
        "<span style=\"text-decoration: underline;\">Delta</span> "
        "<span style=\"text-decoration: line-through;\">Epsilon</span> "
        "<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">Zeta</span>"
        "<br/>Tail");

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            editorHtml),
        QStringLiteral(
            "Alpha <style weight=\"900\">Beta</style> <italic>Gamma</italic> <underline>Delta</underline> "
            "<strikethrough>Epsilon</strikethrough> <highlight>Zeta</highlight>\nTail"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_roundTripsCanonicalStyleTagAttributes()
{
    struct StyleTokenExpectation final
    {
        QString token;
        int pixelSize = 0;
        int weight = 0;
        int lineHeight = 0;
    };

    const QVector<StyleTokenExpectation> styleExpectations = {
        {QStringLiteral("Title"), 26, 700, 26},
        {QStringLiteral("Title2"), 22, 600, 22},
        {QStringLiteral("Subtitle"), 15, 500, 15},
        {QStringLiteral("Header"), 17, 700, 17},
        {QStringLiteral("Header2"), 15, 600, 15},
        {QStringLiteral("Body"), 12, 500, 12},
        {QStringLiteral("Description"), 12, 400, 12},
        {QStringLiteral("Caption"), 11, 600, 11},
        {QStringLiteral("Footnote"), 11, 400, 11}
    };

    for (const StyleTokenExpectation& expectation : styleExpectations)
    {
        const QString tokenSourceText = QStringLiteral("<style style=\"%1\">Token text</style>")
            .arg(expectation.token);
        const QString tokenEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
            QStringLiteral("note"),
            tokenSourceText);

        QVERIFY(tokenEditorHtml.contains(QStringLiteral("font-family:'Pretendard';")));
        QVERIFY(tokenEditorHtml.contains(QStringLiteral("font-size:%1px;").arg(expectation.pixelSize)));
        QVERIFY(tokenEditorHtml.contains(QStringLiteral("font-weight:%1;").arg(expectation.weight)));
        QVERIFY(tokenEditorHtml.contains(QStringLiteral("line-height:%1px;").arg(expectation.lineHeight)));
        QVERIFY(tokenEditorHtml.contains(
            QStringLiteral("<span style=\"font-family:'Pretendard';font-size:%1px;font-weight:%2;line-height:%3px;\">")
                .arg(expectation.pixelSize)
                .arg(expectation.weight)
                .arg(expectation.lineHeight)));
        QCOMPARE(
            WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), tokenEditorHtml),
            tokenSourceText);

        QTextDocument tokenRoundTrippedDocument;
        tokenRoundTrippedDocument.setHtml(tokenEditorHtml);
        const QString tokenRoundTrippedHtml = tokenRoundTrippedDocument.toHtml();
        const QString tokenRecoveredSource = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            tokenRoundTrippedHtml);
        QVERIFY2(
            tokenRecoveredSource == tokenSourceText,
            qPrintable(tokenRoundTrippedHtml));
    }

    const QString bodyFallbackSourceText = QStringLiteral("<style>Body fallback text</style>");
    const QString bodyFallbackEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        bodyFallbackSourceText);
    QVERIFY(bodyFallbackEditorHtml.contains(QStringLiteral("font-family:'Pretendard';")));
    QVERIFY(bodyFallbackEditorHtml.contains(QStringLiteral("font-size:12px;")));
    QVERIFY(bodyFallbackEditorHtml.contains(QStringLiteral("font-weight:500;")));
    QVERIFY(bodyFallbackEditorHtml.contains(QStringLiteral("line-height:12px;")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), bodyFallbackEditorHtml),
        bodyFallbackSourceText);

    const QString sourceText = QStringLiteral(
        "<style style=\"Title\" font=\"Pretendard\" weight=\"600\" size=14 "
        "color=\"#F3F5F8\" background=\"#262728\" align=\"center\" height=1.25>"
        "Styled text</style>");

    const QString bodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);

    QVERIFY(bodyDocument.contains(QStringLiteral("<style style=\"Title\" font=\"Pretendard\" weight=\"600\" size=14")));
    QVERIFY(bodyDocument.contains(QStringLiteral("color=\"#F3F5F8\" background=\"#262728\" align=\"center\" height=1.25")));
    QVERIFY(bodyDocument.contains(QStringLiteral("Styled text</style>")));
    QCOMPARE(WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument), sourceText);

    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        sourceText);
    QVERIFY(editorHtml.contains(QStringLiteral("<!--whatson-style-source:")));
    QVERIFY(editorHtml.contains(QStringLiteral("font-family:'Pretendard';")));
    QVERIFY(editorHtml.contains(QStringLiteral("font-weight:600;")));
    QVERIFY(editorHtml.contains(QStringLiteral("font-size:14px;")));
    QVERIFY(editorHtml.contains(QStringLiteral("color:#F3F5F8;")));
    QVERIFY(editorHtml.contains(QStringLiteral("background-color:#262728;")));
    QVERIFY(editorHtml.contains(QStringLiteral("text-align:center;")));
    QVERIFY(editorHtml.contains(QStringLiteral("line-height:1.25;")));
    QVERIFY(!editorHtml.contains(QStringLiteral("<strong style=\"font-weight:900;\">text</strong>")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), editorHtml),
        sourceText);

    const QString fontTypingSourceText =
        QStringLiteral("Alpha <style font=\"American Typewriter\">Styled</style>");
    QTextDocument fontTypingDocument;
    fontTypingDocument.setHtml(WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        fontTypingSourceText));
    QTextCursor fontTypingCursor(&fontTypingDocument);
    fontTypingCursor.setPosition(QStringLiteral("Alpha Styled").size());
    fontTypingCursor.insertText(QStringLiteral("!"));

    QString typedFamily;
    for (QTextBlock block = fontTypingDocument.begin(); block.isValid(); block = block.next())
    {
        for (QTextBlock::iterator fragmentIt = block.begin(); !fragmentIt.atEnd(); ++fragmentIt)
        {
            const QTextFragment fragment = fragmentIt.fragment();
            if (!fragment.isValid() || !fragment.text().contains(QStringLiteral("!")))
            {
                continue;
            }
            const QVariant typedFamilies = fragment.charFormat().fontFamilies();
            const QStringList typedFamilyList = typedFamilies.toStringList();
            typedFamily = typedFamilyList.isEmpty()
                ? typedFamilies.toString()
                : typedFamilyList.constFirst();
            if (typedFamily.isEmpty())
            {
                typedFamily = fragment.charFormat().font().family();
            }
        }
    }
    QVERIFY2(
        typedFamily == QStringLiteral("American Typewriter"),
        qPrintable(fontTypingDocument.toHtml()));

    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QVERIFY(!editorDocument.toPlainText().contains(QChar(0x200B)));

    const QString fontSourceText =
        QStringLiteral("Alpha <style font=\"American Typewriter\">Styled text</style>");
    const QString fontEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        fontSourceText);
    QVERIFY(fontEditorHtml.contains(QStringLiteral("font-family:'American Typewriter';")));

    QTextDocument fontEditorDocument;
    fontEditorDocument.setHtml(fontEditorHtml);
    QString renderedFamily;
    for (QTextBlock block = fontEditorDocument.begin(); block.isValid(); block = block.next())
    {
        for (QTextBlock::iterator fragmentIt = block.begin(); !fragmentIt.atEnd(); ++fragmentIt)
        {
            const QTextFragment fragment = fragmentIt.fragment();
            if (!fragment.isValid() || !fragment.text().contains(QStringLiteral("tyled text")))
            {
                continue;
            }
            const QVariant renderedFamilies = fragment.charFormat().fontFamilies();
            const QStringList renderedFamilyList = renderedFamilies.toStringList();
            renderedFamily = renderedFamilyList.isEmpty()
                ? renderedFamilies.toString()
                : renderedFamilyList.constFirst();
            if (renderedFamily.isEmpty())
            {
                renderedFamily = fragment.charFormat().font().family();
            }
        }
    }
    QVERIFY2(
        renderedFamily == QStringLiteral("American Typewriter"),
        qPrintable(fontEditorDocument.toHtml()));
    QVERIFY(editorDocument.toPlainText().contains(QStringLiteral("Styled text")));

    QTextDocument roundTrippedEditorDocument;
    roundTrippedEditorDocument.setHtml(editorHtml);
    const QString roundTrippedEditorHtml = roundTrippedEditorDocument.toHtml();
    const QString recoveredSource = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        QStringLiteral("note"),
        roundTrippedEditorHtml);
    QVERIFY2(recoveredSource == sourceText, qPrintable(roundTrippedEditorHtml));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversQtTextEditSerializedStyleAnchors()
{
    const QString qtTextEditHtml = QStringLiteral(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /></head>"
        "<body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;"
        " -qt-block-indent:0; text-indent:0px; line-height:12px;\">"
        "<a name=\"whatson-style-source:3c7374796c65207374796c653d225469746c65223e\"></a>"
        "<span style=\" font-family:'Pretendard'; font-size:26px; font-weight:700; color:rgba(255,255,255,0.8);\">Alpha</span>"
        "<span style=\" font-family:'Pretendard'; font-size:26px; font-weight:700; color:rgba(255,255,255,0.8);\"> Beta</span>"
        "</p></body></html>");

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), qtTextEditHtml),
        QStringLiteral("<style style=\"Title\">Alpha Beta</style>"));

    const QString fontQtTextEditHtml = QStringLiteral(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /></head>"
        "<body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;"
        " -qt-block-indent:0; text-indent:0px;\">"
        "<a name=\"whatson-style-source:3c7374796c6520666f6e743d22416d65726963616e2054797065777269746572223e\"></a>"
        "<span style=\" font-family:'American Typewriter';\">Styled</span>"
        "<span style=\" font-family:'American Typewriter';\"> font</span>"
        "</p></body></html>");
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), fontQtTextEditHtml),
        QStringLiteral("<style font=\"American Typewriter\">Styled font</style>"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesCrossParagraphInlineSourceTagsWithoutEscaping()
{
    const QString crossedInlineSource = QStringLiteral("<bold>첫 줄\n둘째 줄</bold>");
    const QString crossedInlineBodyDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        crossedInlineSource);
    QVERIFY(crossedInlineBodyDocument.contains(QStringLiteral("<paragraph><bold>첫 줄</paragraph>")));
    QVERIFY(crossedInlineBodyDocument.contains(QStringLiteral("<paragraph>둘째 줄</bold></paragraph>")));
    QVERIFY(!crossedInlineBodyDocument.contains(QStringLiteral("&lt;bold&gt;")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(crossedInlineBodyDocument),
        crossedInlineSource);
}

void WhatSonCppRegressionTests::noteBodyPersistence_projectsCalloutAsFigmaBlockAndRecoversSource()
{
    const QString sourceText =
        QStringLiteral(
            "<callout>Alpha <style weight=\"900\">Beta</style> and <italic>Gamma</italic> wraps into the callout body</callout>");

    const QString editorHtml =
        WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), sourceText);

    QVERIFY(editorHtml.contains(QStringLiteral("<div class=\"whatson-callout\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("class=\"whatson-callout\"")));
    QVERIFY(!editorHtml.contains(QStringLiteral("<span class=\"whatson-callout\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-figma-node-id=\"280:7897\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-content=\"true\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-width-mode=\"fill\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-height-mode=\"hug-contents\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-padding-left=\"4\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-padding-top=\"4\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-padding-bottom=\"4\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-width=\"3\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-design-height=\"14\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-height-mode=\"match-content\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-radius=\"3\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-content-gap=\"12\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-frame-min-height=\"22\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-frame-chrome-height=\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("height:auto")));
    QVERIFY(!editorHtml.contains(QStringLiteral("data-frame-design-height")));
    QVERIFY(editorHtml.contains(QStringLiteral("background-color:#262728")));
    QVERIFY(editorHtml.contains(QStringLiteral("padding:4px 4px")));
    QVERIFY(!editorHtml.contains(QStringLiteral("border-left:3px solid #d9d9d9")));
    QVERIFY(editorHtml.contains(QStringLiteral("class=\"whatson-callout-leading-bar\"")));
    QVERIFY(!editorHtml.contains(QStringLiteral("class=\"whatson-callout-content-gap\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-frame-chrome=\"true\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("font-weight:900;")));
    QVERIFY(editorHtml.contains(QStringLiteral(">Beta</span>")));
    QVERIFY(editorHtml.contains(QStringLiteral("<span style=\"font-style:italic;\">Gamma</span>")));
    QVERIFY(!editorHtml.contains(QStringLiteral("<callout>")));

    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QString editorPlainText = editorDocument.toPlainText().trimmed();
    editorPlainText.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(editorPlainText.split(QLatin1Char('\n'), Qt::KeepEmptyParts).size(), 1);
    QVERIFY(editorPlainText.contains(QStringLiteral("Alpha Beta and Gamma wraps into the callout body")));

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), editorHtml),
        sourceText);

    QTextDocument roundTrippedEditorDocument;
    roundTrippedEditorDocument.setHtml(editorHtml);
    const QString roundTrippedEditorHtml = roundTrippedEditorDocument.toHtml();
    QVERIFY(!roundTrippedEditorHtml.contains(QStringLiteral("whatson-callout-source:")));
    QVERIFY(!roundTrippedEditorHtml.contains(QStringLiteral("data-callout-content")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), roundTrippedEditorHtml),
        sourceText);
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesExplicitBlankLineBeforeStandaloneCallout()
{
    const QString sourceText =
        QStringLiteral("Before\n"
                       "\n"
                       "<callout>Inside</callout>\n"
                       "After");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-explicit-blank-line-note"),
        sourceText);
    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QString editorVisiblePlainText = editorDocument.toPlainText();
    editorVisiblePlainText.remove(QChar::ObjectReplacementCharacter);
    editorVisiblePlainText.remove(QChar(0x200B));
    QCOMPARE(editorVisiblePlainText, QStringLiteral("Before\n\nInside\nAfter"));

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("callout-explicit-blank-line-note"),
            editorHtml),
        sourceText);
}

void WhatSonCppRegressionTests::noteBodyPersistence_doesNotReplicateParagraphsAroundRepeatedCalloutSaves()
{
    const QString sourceText =
        QStringLiteral("Before\n"
                       "<callout>Inside <style weight=\"900\">callout</style></callout>\n"
                       "After");
    QString editorHtml =
        WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), sourceText);

    for (int cycle = 0; cycle < 4; ++cycle)
    {
        QTextDocument editorDocument;
        editorDocument.setHtml(editorHtml);
        const QString qtSerializedEditorHtml = editorDocument.toHtml();
        const QString recoveredSource =
            WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
                QStringLiteral("note"),
                qtSerializedEditorHtml);

        QCOMPARE(recoveredSource, sourceText);
        QCOMPARE(recoveredSource.split(QLatin1Char('\n'), Qt::KeepEmptyParts).size(), 3);
        QVERIFY(!recoveredSource.contains(QStringLiteral("\n\n<callout>")));
        QVERIFY(!recoveredSource.contains(QStringLiteral("</callout>\n\n")));

        editorHtml =
            WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), recoveredSource);
    }
}

void WhatSonCppRegressionTests::noteBodyPersistence_persistsCalloutAsParagraphTag()
{
    const QString sourceText =
        QStringLiteral("<callout>Alpha & <bold>Beta</bold></callout>");
    const QString bodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);

    QVERIFY(bodyDocument.contains(
        QStringLiteral("    <paragraph><callout>Alpha &amp; <bold>Beta</bold></callout></paragraph>\n")));
    QVERIFY(!bodyDocument.contains(QStringLiteral("    <callout>")));
    QVERIFY(!bodyDocument.contains(QStringLiteral("&lt;callout")));

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument),
        sourceText);
    QCOMPARE(
        WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(bodyDocument),
        QStringLiteral("Alpha & Beta"));

    const QString legacyParagraphWrappedBodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <paragraph>&lt;callout&gt;Legacy&lt;/callout&gt;</paragraph>\n"
        "  </body>\n"
        "</contents>\n");
    const QString recoveredSource =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(legacyParagraphWrappedBodyDocument);
    QCOMPARE(recoveredSource, QStringLiteral("<callout>Legacy</callout>"));
    const QString recoveredBodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), recoveredSource);
    QVERIFY(recoveredBodyDocument.contains(QStringLiteral("    <paragraph><callout>Legacy</callout></paragraph>\n")));
    QVERIFY(!recoveredBodyDocument.contains(QStringLiteral("    <callout>Legacy</callout>\n")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_stripsRenderedHtmlBlockArtifactsFromSourceProjection()
{
    const QString bodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <!--whatson-resource-block:0--><p style=\"margin-top:0px;margin-bottom:0px;\">alpha</p><!--/whatson-resource-block:0-->\n"
        "  </body>\n"
        "</contents>\n");

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument);
    QCOMPARE(sourceText, QStringLiteral("alpha"));
    QVERIFY(!sourceText.contains(QStringLiteral("whatson-resource-block")));
    QVERIFY(!sourceText.contains(QStringLiteral("<p")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversRenderedResourceFrameMarkersAsSourceTags()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    const QString encodedResourceTag = QString::fromLatin1(resourceTag.toUtf8().toHex());
    const QString renderedEditorHtml = QStringLiteral(
                                           "<p>Alpha</p>"
                                           "<!--whatson-resource-source:%1-->"
                                           "<table class=\"whatson-resource-frame\" data-resource-preview=\"image-only-frame\">"
                                           "<tr><td><img class=\"whatson-resource-media\" src=\"data:image/png;base64,ZmFrZQ==\" /></td></tr>"
                                           "</table>"
                                           "<!--/whatson-resource-source-->"
                                           "<p>Beta</p>")
                                           .arg(encodedResourceTag);

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        QStringLiteral("resource-note"),
        renderedEditorHtml);

    QCOMPARE(
        sourceText,
        QStringLiteral("Alpha\n%1\nBeta").arg(resourceTag));
}

void WhatSonCppRegressionTests::noteBodyPersistence_dropsDeletedSingleResourceObjectMarker()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    const QString encodedResourceTag = QString::fromLatin1(resourceTag.toUtf8().toHex());
    const QString renderedEditorHtml = QStringLiteral(
                                           "<p>Alpha</p>"
                                           "<!--whatson-resource-source:%1-->"
                                           "<!--/whatson-resource-source-->"
                                           "<p>Beta</p>")
                                           .arg(encodedResourceTag);

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        QStringLiteral("resource-note"),
        renderedEditorHtml);

    QCOMPARE(sourceText, QStringLiteral("Alpha\nBeta"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesIntentionalBlankLinesAroundRenderedResourceFrame()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    const QString encodedResourceTag = QString::fromLatin1(resourceTag.toUtf8().toHex());
    const QString renderedEditorHtml = QStringLiteral(
                                           "<p>Alpha</p>"
                                           "<p><br /></p>"
                                           "<!--whatson-resource-source:%1-->"
                                           "<table class=\"whatson-resource-frame\" data-resource-preview=\"image-only-frame\">"
                                           "<tr><td><img class=\"whatson-resource-media\" src=\"data:image/png;base64,ZmFrZQ==\" /></td></tr>"
                                           "</table>"
                                           "<!--/whatson-resource-source-->"
                                           "<p><br /></p>"
                                           "<p>Beta</p>")
                                           .arg(encodedResourceTag);

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        QStringLiteral("resource-note"),
        renderedEditorHtml);

    QCOMPARE(
        sourceText,
        QStringLiteral("Alpha\n\n%1\n\nBeta").arg(resourceTag));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesEmptyParagraphCursorLineAfterResource()
{
    const QString bodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <resource type=\"image\" path=\"cover.png\" />\n"
        "    <paragraph></paragraph>\n"
        "  </body>\n"
        "</contents>\n");

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument);
    QCOMPARE(sourceText, QStringLiteral("<resource type=\"image\" path=\"cover.png\" />\n"));

    const QString roundTripDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);
    QVERIFY(roundTripDocument.contains(
        QStringLiteral("<resource type=\"image\" path=\"cover.png\" />\n"
                       "    <paragraph></paragraph>")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesEmptyParagraphBoundariesAroundResources()
{
    const QString leadingEmptyParagraphBodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <paragraph></paragraph>\n"
        "    <resource type=\"image\" path=\"cover.png\" />\n"
        "  </body>\n"
        "</contents>\n");
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(leadingEmptyParagraphBodyDocument),
        QStringLiteral("\n<resource type=\"image\" path=\"cover.png\" />"));

    const QString interiorEmptyParagraphBodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <resource type=\"image\" path=\"first.png\" />\n"
        "    <paragraph></paragraph>\n"
        "    <resource type=\"image\" path=\"second.png\" />\n"
        "  </body>\n"
        "</contents>\n");
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(interiorEmptyParagraphBodyDocument),
        QStringLiteral(
            "<resource type=\"image\" path=\"first.png\" />\n"
            "\n"
            "<resource type=\"image\" path=\"second.png\" />"));
}
