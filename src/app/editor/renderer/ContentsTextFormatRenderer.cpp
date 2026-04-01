#include "ContentsTextFormatRenderer.hpp"

#include <QRegularExpression>
#include <QStringList>

namespace
{
    QString escapeHtmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }

    QString normalizeLineEndings(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return text;
    }

    QString normalizedInlineStyleTagName(const QString& elementName)
    {
        const QString normalizedName = elementName.trimmed().toCaseFolded();
        if (normalizedName == QStringLiteral("bold")
            || normalizedName == QStringLiteral("b")
            || normalizedName == QStringLiteral("strong"))
        {
            return QStringLiteral("strong");
        }
        if (normalizedName == QStringLiteral("italic")
            || normalizedName == QStringLiteral("i")
            || normalizedName == QStringLiteral("em"))
        {
            return QStringLiteral("em");
        }
        if (normalizedName == QStringLiteral("underline")
            || normalizedName == QStringLiteral("u"))
        {
            return QStringLiteral("u");
        }
        if (normalizedName == QStringLiteral("strikethrough")
            || normalizedName == QStringLiteral("strike")
            || normalizedName == QStringLiteral("s")
            || normalizedName == QStringLiteral("del"))
        {
            return QStringLiteral("s");
        }
        return {};
    }

    bool isClosingTagToken(const QString& token)
    {
        for (int index = 1; index < token.size(); ++index)
        {
            const QChar ch = token.at(index);
            if (!ch.isSpace())
            {
                return ch == QLatin1Char('/');
            }
        }
        return false;
    }

    bool isSelfClosingTagToken(QString token)
    {
        token = token.trimmed();
        return token.endsWith(QStringLiteral("/>"));
    }

    void closeMatchingTag(
        QStringList* openStyleTags,
        QString* htmlOutput,
        const QString& normalizedStyleTag)
    {
        if (openStyleTags == nullptr || htmlOutput == nullptr || normalizedStyleTag.isEmpty())
        {
            return;
        }

        const int openTagIndex = openStyleTags->lastIndexOf(normalizedStyleTag);
        if (openTagIndex < 0)
        {
            return;
        }

        for (int index = openStyleTags->size() - 1; index >= openTagIndex; --index)
        {
            const QString openTag = openStyleTags->at(index);
            *htmlOutput += QStringLiteral("</%1>").arg(openTag);
            openStyleTags->removeAt(index);
        }
    }

    QString renderInlineTaggedTextToHtml(const QString& sourceText)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        if (normalizedText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

        QString html;
        QStringList openStyleTags;
        qsizetype cursor = 0;

        QRegularExpressionMatchIterator iterator = tagPattern.globalMatch(normalizedText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            const qsizetype tagStart = match.capturedStart(0);
            const qsizetype tagEnd = match.capturedEnd(0);
            if (tagStart < 0 || tagEnd <= tagStart)
            {
                continue;
            }

            if (tagStart > cursor)
            {
                html += escapeHtmlText(normalizedText.mid(cursor, tagStart - cursor));
            }

            const QString fullTagToken = match.captured(0);
            const QString rawTagName = match.captured(1);
            const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
            const bool closingTag = isClosingTagToken(fullTagToken);
            const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);

            if (normalizedTagName == QStringLiteral("resource"))
            {
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("br"))
            {
                html += QStringLiteral("<br/>");
                cursor = tagEnd;
                continue;
            }

            const QString styleTag = normalizedInlineStyleTagName(rawTagName);
            if (!styleTag.isEmpty())
            {
                if (closingTag)
                {
                    closeMatchingTag(&openStyleTags, &html, styleTag);
                }
                else if (!selfClosingTag)
                {
                    html += QStringLiteral("<%1>").arg(styleTag);
                    openStyleTags.append(styleTag);
                }
                cursor = tagEnd;
                continue;
            }

            html += escapeHtmlText(fullTagToken);
            cursor = tagEnd;
        }

        if (cursor < normalizedText.size())
        {
            html += escapeHtmlText(normalizedText.mid(cursor));
        }

        for (int index = openStyleTags.size() - 1; index >= 0; --index)
        {
            html += QStringLiteral("</%1>").arg(openStyleTags.at(index));
        }

        html.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
        return html;
    }
} // namespace

ContentsTextFormatRenderer::ContentsTextFormatRenderer(QObject* parent)
    : QObject(parent)
{
}

ContentsTextFormatRenderer::~ContentsTextFormatRenderer() = default;

QString ContentsTextFormatRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsTextFormatRenderer::setSourceText(const QString& sourceText)
{
    if (m_sourceText == sourceText)
    {
        return;
    }

    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedHtml();
}

QString ContentsTextFormatRenderer::renderedHtml() const
{
    return m_renderedHtml;
}

QString ContentsTextFormatRenderer::renderRichText(const QString& sourceText) const
{
    return renderInlineTaggedTextToHtml(sourceText);
}

void ContentsTextFormatRenderer::requestRenderRefresh()
{
    refreshRenderedHtml();
}

void ContentsTextFormatRenderer::refreshRenderedHtml()
{
    const QString nextRenderedHtml = renderInlineTaggedTextToHtml(m_sourceText);
    if (m_renderedHtml == nextRenderedHtml)
    {
        return;
    }

    m_renderedHtml = nextRenderedHtml;
    emit renderedHtmlChanged();
}
