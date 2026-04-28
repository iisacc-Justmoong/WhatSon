#include "app/models/editor/format/ContentsTextFormatRendererInternal.hpp"

#include "app/models/file/note/WhatSonNoteMarkdownStyleObject.hpp"

#include <array>
#include <QRegularExpression>

namespace
{
    using WhatSon::ContentsTextFormatRendererInternal::LiteralRenderMode;

    QString normalizeLineEndings(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar::LineSeparator, QLatin1Char('\n'));
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        text.replace(QChar::Nbsp, QLatin1Char(' '));
        return text;
    }

    QString escapeHtmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }

    QString decodeHtmlEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    QString whitespaceToHtml(const QString& value)
    {
        const QString decodedValue = decodeHtmlEntities(value);
        QString html;
        html.reserve(decodedValue.size() * 6);
        for (const QChar ch : decodedValue)
        {
            if (ch == QLatin1Char(' '))
            {
                html += QStringLiteral("&nbsp;");
            }
            else if (ch == QLatin1Char('\t'))
            {
                html += QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;");
            }
            else
            {
                html += escapeHtmlText(QString(ch));
            }
        }
        return html;
    }
}

namespace WhatSon::ContentsTextFormatRendererInternal
{
    QString renderMarkdownAwareTextToHtml(const QString& sourceText)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        if (normalizedText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression unorderedListPattern(
            QStringLiteral(R"(^([ \t]*)(?:([-+*])|(•))(\s+)(.*)$)"));
        static const QRegularExpression orderedListPattern(
            QStringLiteral(R"(^([ \t]*)(\d+)([.)])(\s+)(.*)$)"));
        static const QRegularExpression headingPattern(
            QStringLiteral(R"(^([ \t]*)(#{1,6})(\s+)(.*)$)"));
        static const QRegularExpression blockquotePattern(
            QStringLiteral(R"(^([ \t]*)(>)(\s?)(.*)$)"));
        static const QRegularExpression codeFencePattern(
            QStringLiteral(R"(^([ \t]*)```(.*)$)"));
        static const QRegularExpression horizontalRulePattern(
            QStringLiteral(R"(^[ \t]{0,3}(([-*_])(?:[ \t]*\2){2,})[ \t]*$)"));

        const QStringList lines = normalizedText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QStringList htmlLines;
        htmlLines.reserve(lines.size());

        bool insideCodeFence = false;
        for (const QString& line : lines)
        {
            const QRegularExpressionMatch codeFenceMatch = codeFencePattern.match(line);
            if (codeFenceMatch.hasMatch())
            {
                htmlLines.push_back(
                    whitespaceToHtml(codeFenceMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::CodeFence,
                        escapeHtmlText(decodeHtmlEntities(line.mid(codeFenceMatch.capturedLength(1))))));
                insideCodeFence = !insideCodeFence;
                continue;
            }

            if (insideCodeFence)
            {
                htmlLines.push_back(
                    WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::CodeBody,
                        whitespaceToHtml(line)));
                continue;
            }

            const QRegularExpressionMatch unorderedListMatch = unorderedListPattern.match(line);
            if (unorderedListMatch.hasMatch())
            {
                const QString markerToken = !unorderedListMatch.captured(2).isEmpty()
                                                ? unorderedListMatch.captured(2)
                                                : unorderedListMatch.captured(3);
                const QString markerSpacing = unorderedListMatch.captured(4);
                const QString trailingSpacing = markerSpacing.size() > 1
                                                    ? whitespaceToHtml(markerSpacing.mid(1))
                                                    : QString();
                htmlLines.push_back(
                    whitespaceToHtml(unorderedListMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::UnorderedListMarker,
                        escapeHtmlText(markerToken))
                    + QStringLiteral("&nbsp;")
                    + trailingSpacing
                    + renderInlineTaggedTextFragmentToHtml(unorderedListMatch.captured(5)));
                continue;
            }

            const QRegularExpressionMatch orderedListMatch = orderedListPattern.match(line);
            if (orderedListMatch.hasMatch())
            {
                const QString markerToken =
                    orderedListMatch.captured(2) + orderedListMatch.captured(3);
                htmlLines.push_back(
                    whitespaceToHtml(orderedListMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::OrderedListMarker,
                        escapeHtmlText(markerToken))
                    + whitespaceToHtml(orderedListMatch.captured(4))
                    + renderInlineTaggedTextFragmentToHtml(orderedListMatch.captured(5)));
                continue;
            }

            const QRegularExpressionMatch headingMatch = headingPattern.match(line);
            if (headingMatch.hasMatch())
            {
                const int headingLevel = headingMatch.captured(2).size();
                htmlLines.push_back(
                    whitespaceToHtml(headingMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::HeadingMarker,
                        escapeHtmlText(headingMatch.captured(2) + headingMatch.captured(3)))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::HeadingBody,
                        renderInlineTaggedTextFragmentToHtml(headingMatch.captured(4)),
                        headingLevel));
                continue;
            }

            const QRegularExpressionMatch blockquoteMatch = blockquotePattern.match(line);
            if (blockquoteMatch.hasMatch())
            {
                htmlLines.push_back(
                    whitespaceToHtml(blockquoteMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::BlockquoteMarker,
                        escapeHtmlText(blockquoteMatch.captured(2)))
                    + whitespaceToHtml(blockquoteMatch.captured(3))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::BlockquoteBody,
                        renderInlineTaggedTextFragmentToHtml(blockquoteMatch.captured(4))));
                continue;
            }

            if (horizontalRulePattern.match(line).hasMatch())
            {
                htmlLines.push_back(
                    WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::HorizontalRule,
                        escapeHtmlText(line)));
                continue;
            }

            htmlLines.push_back(renderInlineTaggedTextFragmentToHtml(line));
        }

        return htmlLines.join(QStringLiteral("<br/>"));
    }

    QString applyPaperPaletteToHtml(QString html)
    {
        if (html.isEmpty())
        {
            return html;
        }

        static const std::array<std::pair<QString, QString>, 10> replacements{{
            {
                QStringLiteral("background-color:#8A4B00;color:#D6AE58;font-weight:600;"),
                QStringLiteral("background-color:#F4D37A;color:#111111;font-weight:600;")
            },
            {
                QStringLiteral("background-color:#1A1D22;color:#D7DADF;"),
                QStringLiteral("background-color:#E7EAEE;color:#111111;")
            },
            {
                QStringLiteral("background-color:#1A1D22;color:#8F96A3;"),
                QStringLiteral("background-color:#E7EAEE;color:#4E5763;")
            },
            {QStringLiteral("color:#F3F5F8;"), QStringLiteral("color:#111111;")},
            {QStringLiteral("color:#C9CDD4;"), QStringLiteral("color:#2F343B;")},
            {QStringLiteral("color:#8F96A3;"), QStringLiteral("color:#4E5763;")},
            {QStringLiteral("color:#8CB4FF;"), QStringLiteral("color:#1F5FBF;")},
            {QStringLiteral("color:#66727D;"), QStringLiteral("color:#4E5763;")},
            {QStringLiteral("color:#D6AE58;"), QStringLiteral("color:#111111;")},
            {QStringLiteral("color:#D7DADF;"), QStringLiteral("color:#111111;")},
        }};

        for (const auto& [from, to] : replacements)
        {
            html.replace(from, to);
        }

        return html;
    }

    QVariantList applyPaperPaletteToHtmlField(const QVariantList& entries, const QString& fieldName)
    {
        QVariantList recoloredEntries;
        recoloredEntries.reserve(entries.size());

        for (const QVariant& entryValue : entries)
        {
            QVariantMap entry = entryValue.toMap();
            const QString currentHtml = entry.value(fieldName).toString();
            if (!currentHtml.isEmpty())
            {
                entry.insert(fieldName, applyPaperPaletteToHtml(currentHtml));
            }
            recoloredEntries.push_back(entry);
        }

        return recoloredEntries;
    }
}
