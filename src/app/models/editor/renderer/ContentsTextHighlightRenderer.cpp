#include "ContentsTextHighlightRenderer.hpp"

namespace
{
    // LVRS Theme.accentYellowMuted
    constexpr auto kHighlightOpenHtmlTag = "<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">";
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
