#include "ContentsCalloutBackend.hpp"

#include <QRegularExpression>

namespace
{
    QString normalizePlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar(0x2028), QLatin1Char('\n'));
        text.replace(QChar(0x2029), QLatin1Char('\n'));
        text.replace(QChar(0x00A0), QLatin1Char(' '));
        return text;
    }

    QString escapeSourceLiteral(QString text)
    {
        text = normalizePlainText(text);
        text.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        text.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        text.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        text.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        text.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return text;
    }

    QString decodeSourceEntities(QString text)
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

    QString visibleCalloutText(QString rawCalloutInnerText)
    {
        rawCalloutInnerText = normalizePlainText(rawCalloutInnerText);
        rawCalloutInnerText.replace(
            QRegularExpression(
                QStringLiteral(R"(<\s*br\s*/?\s*>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("\n"));
        rawCalloutInnerText.remove(QRegularExpression(QStringLiteral(R"(<[^>]*>)")));
        rawCalloutInnerText = decodeSourceEntities(rawCalloutInnerText);
        return normalizePlainText(rawCalloutInnerText).trimmed();
    }
} // namespace

ContentsCalloutBackend::ContentsCalloutBackend(QObject* parent)
    : QObject(parent)
{
}

ContentsCalloutBackend::~ContentsCalloutBackend() = default;

QVariantList ContentsCalloutBackend::parseCallouts(const QString& sourceText) const
{
    QVariantList callouts;
    if (sourceText.isEmpty())
    {
        return callouts;
    }

    static const QRegularExpression calloutPattern(
        QStringLiteral(R"(<callout\b[^>]*>([\s\S]*?)</callout>)"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator iterator = calloutPattern.globalMatch(sourceText);
    while (iterator.hasNext())
    {
        const QRegularExpressionMatch match = iterator.next();
        if (!match.hasMatch())
        {
            continue;
        }

        QVariantMap entry;
        entry.insert(
            QStringLiteral("text"),
            visibleCalloutText(match.captured(1)));
        callouts.push_back(entry);
    }

    return callouts;
}

QVariantMap ContentsCalloutBackend::buildCalloutInsertionPayload(const QString& bodyText) const
{
    const QString escapedBodyText = escapeSourceLiteral(bodyText);
    const QString calloutOpenTag = QStringLiteral("<callout>");
    const QString calloutCloseTag = QStringLiteral("</callout>");

    QVariantMap insertionPayload;
    insertionPayload.insert(QStringLiteral("applied"), true);
    insertionPayload.insert(QStringLiteral("calloutOpenTag"), calloutOpenTag);
    insertionPayload.insert(QStringLiteral("calloutCloseTag"), calloutCloseTag);
    insertionPayload.insert(
        QStringLiteral("cursorSourceOffsetFromInsertionStart"),
        calloutOpenTag.size() + escapedBodyText.size());
    insertionPayload.insert(
        QStringLiteral("insertionSourceText"),
        calloutOpenTag + escapedBodyText + calloutCloseTag);
    return insertionPayload;
}
