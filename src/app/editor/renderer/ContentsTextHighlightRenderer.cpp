#include "ContentsTextHighlightRenderer.hpp"

namespace
{
    constexpr auto kHighlightStackTagName = "span";
    constexpr auto kHighlightOpenHtmlTag = "<span style=\"background-color:#8A4B00;color:#FFD9A3;font-weight:600;\">";
    constexpr auto kHighlightCloseHtmlTag = "</span>";
}

bool ContentsTextHighlightRenderer::isHighlightTagAlias(const QString& elementName)
{
    const QString normalizedName = elementName.trimmed().toCaseFolded();
    return normalizedName == QStringLiteral("highlight")
        || normalizedName == QStringLiteral("mark");
}

QString ContentsTextHighlightRenderer::highlightOpenHtmlTag()
{
    return QString::fromLatin1(kHighlightOpenHtmlTag);
}

QString ContentsTextHighlightRenderer::highlightCloseHtmlTag()
{
    return QString::fromLatin1(kHighlightCloseHtmlTag);
}

QString ContentsTextHighlightRenderer::highlightStackTagName()
{
    return QString::fromLatin1(kHighlightStackTagName);
}
