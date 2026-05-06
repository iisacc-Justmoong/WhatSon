#include "test/cpp/whatson_cpp_regression_tests.hpp"

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
