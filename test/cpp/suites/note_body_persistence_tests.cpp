#include "../whatson_cpp_regression_tests.hpp"

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

    const QString autoWrappedDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("Visit www.iisacc.com"));
    QVERIFY(autoWrappedDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink>")));
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
