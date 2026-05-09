#include "app/models/editor/format/ContentsCalloutHtmlRenderer.hpp"

#include <QRegularExpression>

namespace
{
    QString fallbackCalloutBodyHtml(const QString& innerHtml)
    {
        return innerHtml.trimmed().isEmpty() ? QStringLiteral("&nbsp;") : innerHtml;
    }
} // namespace

namespace WhatSon::ContentsCalloutHtmlRenderer
{
bool containsCalloutTag(const QString& sourceText)
{
    static const QRegularExpression calloutTagPattern(
        QStringLiteral(R"(<\s*/?\s*callout\b)"),
        QRegularExpression::CaseInsensitiveOption);
    return calloutTagPattern.match(sourceText).hasMatch();
}

CalloutSourceSpan singleCalloutSpan(const QString& sourceText)
{
    static const QRegularExpression singleCalloutPattern(
        QStringLiteral(R"(^\s*<\s*callout\b[^>]*>([\s\S]*?)<\s*/\s*callout\s*>\s*$)"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = singleCalloutPattern.match(sourceText);
    if (!match.hasMatch())
    {
        return {};
    }

    CalloutSourceSpan span;
    span.innerSource = match.captured(1);
    if (containsCalloutTag(span.innerSource))
    {
        return {};
    }
    span.innerStart = match.capturedStart(1);
    span.innerEnd = match.capturedEnd(1);
    span.valid = true;
    return span;
}

QString renderCalloutBlockHtml(const QString& innerHtml)
{
    return QStringLiteral(
        "<table cellspacing=\"0\" cellpadding=\"0\" border=\"0\" width=\"100%\" bgcolor=\"#262728\" "
        "style=\"margin-top:0px;margin-bottom:0px;background-color:#262728;width:100%;\">"
        "<tr><td colspan=\"5\" height=\"4\" style=\"height:4px;font-size:1px;line-height:4px;\"></td></tr>"
        "<tr>"
        "<td width=\"4\" style=\"width:4px;font-size:1px;line-height:1px;\"></td>"
        "<td width=\"3\" bgcolor=\"#d9d9d9\" "
        "style=\"width:3px;min-width:3px;background-color:#d9d9d9;"
        "font-size:1px;line-height:1px;\"></td>"
        "<td width=\"12\" style=\"width:12px;font-size:1px;line-height:1px;\"></td>"
        "<td valign=\"top\" style=\"font-family:Pretendard;font-size:12px;font-weight:500;line-height:12px;"
        "color:#ffffff;\">%1</td>"
        "<td width=\"4\" style=\"width:4px;font-size:1px;line-height:1px;\"></td>"
        "</tr>"
        "<tr><td colspan=\"5\" height=\"4\" style=\"height:4px;font-size:1px;line-height:4px;\"></td></tr>"
        "</table>")
        .arg(fallbackCalloutBodyHtml(innerHtml));
}
} // namespace WhatSon::ContentsCalloutHtmlRenderer
