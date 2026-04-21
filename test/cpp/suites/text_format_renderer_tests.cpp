#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::textFormatRenderer_appliesPaperPaletteToEditorAndPreviewHtml()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(
        QStringLiteral(
            "<title>Heading</title>\n"
            "<highlight>Glow</highlight>\n"
            "# Print title\n"
            "> Quote\n"
            "`Code`\n"
            "[Doc](https://example.com)"));
    renderer.setPreviewEnabled(true);

    const QString screenEditorHtml = renderer.editorSurfaceHtml();
    const QString screenPreviewHtml = renderer.renderedHtml();
    QVERIFY(screenEditorHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(screenEditorHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#8CB4FF")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#D6AE58")));

    renderer.setPaperPaletteEnabled(true);

    const QString paperEditorHtml = renderer.editorSurfaceHtml();
    const QString paperPreviewHtml = renderer.renderedHtml();
    QVERIFY(!paperEditorHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(!paperEditorHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#8CB4FF")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(paperEditorHtml.contains(QStringLiteral("color:#111111")));
    QVERIFY(paperEditorHtml.contains(QStringLiteral("background-color:#F4D37A")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("color:#111111")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("color:#1F5FBF")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("background-color:#E7EAEE")));
}

void WhatSonCppRegressionTests::textFormatRenderer_wrapsCommittedUrlsIntoCanonicalWebLinks()
{
    ContentsTextFormatRenderer renderer;

    const QString notYetCommittedSource = renderer.applyPlainTextReplacementToSource(
        QStringLiteral("www.iisacc.co"),
        QStringLiteral("www.iisacc.co").size(),
        QStringLiteral("www.iisacc.co").size(),
        QStringLiteral("m"));
    QCOMPARE(notYetCommittedSource, QStringLiteral("www.iisacc.com"));

    const QString committedSource = renderer.applyPlainTextReplacementToSource(
        notYetCommittedSource,
        notYetCommittedSource.size(),
        notYetCommittedSource.size(),
        QStringLiteral(" "));
    QCOMPARE(
        committedSource,
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink> "));

    const QString pastedSource = renderer.applyPlainTextReplacementToSource(
        QString(),
        0,
        0,
        QStringLiteral("Visit https://www.iisacc.com/path?q=1"));
    QCOMPARE(
        pastedSource,
        QStringLiteral(
            "Visit <weblink href=\"https://www.iisacc.com/path?q=1\">https://www.iisacc.com/path?q=1</weblink>"));

    const QString markdownSource = renderer.applyPlainTextReplacementToSource(
        QString(),
        0,
        0,
        QStringLiteral("[Doc](https://example.com)"));
    QCOMPARE(markdownSource, QStringLiteral("[Doc](https://example.com)"));

    renderer.setSourceText(QStringLiteral("<weblink href=\"www.iisacc.com\">아이작닷컴</weblink>"));
    renderer.setPreviewEnabled(true);
    QVERIFY(renderer.renderedHtml().contains(
        QStringLiteral("<a href=\"https://www.iisacc.com\" style=\"color:#8CB4FF;text-decoration: underline;\">")));
    QVERIFY(renderer.renderedHtml().contains(QStringLiteral("아이작닷컴</a>")));
}

void WhatSonCppRegressionTests::textFormatRenderer_preservesMarkdownUnorderedListMarkersWithoutRegexWarnings()
{
    ContentsTextFormatRenderer renderer;
    renderer.setPreviewEnabled(true);
    renderer.setSourceText(QStringLiteral("- alpha\n+ beta\n* gamma\n• delta"));

    const QString previewHtml = renderer.renderedHtml();
    QVERIFY(previewHtml.contains(QStringLiteral(">-</span>&nbsp;alpha")));
    QVERIFY(previewHtml.contains(QStringLiteral(">+</span>&nbsp;beta")));
    QVERIFY(previewHtml.contains(QStringLiteral(">*</span>&nbsp;gamma")));
    QVERIFY(previewHtml.contains(QStringLiteral(">•</span>&nbsp;delta")));
}
