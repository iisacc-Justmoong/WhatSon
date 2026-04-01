#include "editor/renderer/ContentsTextFormatRenderer.hpp"

#include <QtTest/QtTest>

#include <type_traits>

static_assert(
    !std::is_final_v<ContentsTextFormatRenderer>,
    "ContentsTextFormatRenderer must remain non-final for qmlRegisterType wrapper generation.");

class ContentsTextFormatRendererTest final : public QObject
{
    Q_OBJECT

private slots:
    void renderer_mustMapSupportedInlineStyleTags();
    void renderer_mustSupportAliasTagsAndLineBreaks();
    void renderer_mustIgnoreResourceTagsAndEscapeUnknownTags();
};

void ContentsTextFormatRendererTest::renderer_mustMapSupportedInlineStyleTags()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(QStringLiteral("Hello <bold>World</bold> and <italic>Qt</italic>."));

    QCOMPARE(
        renderer.renderedHtml(),
        QStringLiteral("Hello <strong>World</strong> and <em>Qt</em>."));
}

void ContentsTextFormatRendererTest::renderer_mustSupportAliasTagsAndLineBreaks()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(QStringLiteral("<b>A</b>\n<u>B</u>\n<s>C</s>"));

    QCOMPARE(
        renderer.renderedHtml(),
        QStringLiteral("<strong>A</strong><br/><u>B</u><br/><s>C</s>"));
}

void ContentsTextFormatRendererTest::renderer_mustIgnoreResourceTagsAndEscapeUnknownTags()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(
        QStringLiteral("before <resource type=\"image\" path=resources/image.wsresource> <custom>tag</custom> after"));

    const QString renderedHtml = renderer.renderedHtml();
    QVERIFY(!renderedHtml.contains(QStringLiteral("resource type")));
    QVERIFY(renderedHtml.contains(QStringLiteral("&lt;custom&gt;tag&lt;/custom&gt;")));
    QVERIFY(renderedHtml.contains(QStringLiteral("before")));
    QVERIFY(renderedHtml.contains(QStringLiteral("after")));
}

QTEST_MAIN(ContentsTextFormatRendererTest)

#include "test_contents_text_format_renderer.moc"
