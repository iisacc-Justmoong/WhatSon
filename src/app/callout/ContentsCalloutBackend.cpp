#include "ContentsCalloutBackend.hpp"

#include <QRegularExpression>
#include <algorithm>
#include <limits>
#include <optional>

namespace
{
    struct CalloutContext final
    {
        int calloutCloseEnd = 0;
        int calloutCloseStart = 0;
        int calloutContentStart = 0;
    };

    QVariantMap notAppliedResult()
    {
        return {
            {QStringLiteral("applied"), false}
        };
    }

    int boundedQStringSize(const QString& text)
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        return static_cast<int>(std::min<qsizetype>(text.size(), maxIntSize));
    }

    int boundedQSizeToInt(const qsizetype value)
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        if (value < 0)
        {
            return -1;
        }
        if (value > maxIntSize)
        {
            return std::numeric_limits<int>::max();
        }
        return static_cast<int>(value);
    }

    int boundedTextIndex(const QString& text, const int index)
    {
        return std::clamp(index, 0, boundedQStringSize(text));
    }

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

    std::optional<CalloutContext> calloutContextAtSourceOffset(
        const QString& sourceText,
        const int sourceOffset)
    {
        const QString normalizedSourceText = sourceText;
        const int boundedOffset = boundedTextIndex(normalizedSourceText, sourceOffset);
        const QString beforeOffsetText = normalizedSourceText.left(boundedOffset);

        const int calloutOpenStart = beforeOffsetText.lastIndexOf(QStringLiteral("<callout"));
        if (calloutOpenStart < 0)
        {
            return std::nullopt;
        }
        const int calloutOpenEnd = normalizedSourceText.indexOf(QLatin1Char('>'), calloutOpenStart);
        if (calloutOpenEnd < 0)
        {
            return std::nullopt;
        }
        const int calloutContentStart = calloutOpenEnd + 1;
        const int calloutCloseStart =
            normalizedSourceText.indexOf(QStringLiteral("</callout>"), calloutContentStart);
        if (calloutCloseStart < 0
            || boundedOffset < calloutContentStart
            || boundedOffset > calloutCloseStart)
        {
            return std::nullopt;
        }

        CalloutContext context;
        context.calloutCloseEnd = calloutCloseStart + QStringLiteral("</callout>").size();
        context.calloutCloseStart = calloutCloseStart;
        context.calloutContentStart = calloutContentStart;
        return context;
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

        const int calloutStart = std::max(0, boundedQSizeToInt(match.capturedStart(0)));
        const QString calloutToken = match.captured(0);
        const int openTokenEndOffset = calloutToken.indexOf(QLatin1Char('>'));
        const int openTokenLength = openTokenEndOffset >= 0 ? openTokenEndOffset + 1 : 0;

        QVariantMap entry;
        entry.insert(
            QStringLiteral("sourceStart"),
            calloutStart);
        entry.insert(
            QStringLiteral("focusSourceOffset"),
            calloutStart + openTokenLength);
        entry.insert(
            QStringLiteral("text"),
            visibleCalloutText(match.captured(1)));
        callouts.push_back(entry);
    }

    return callouts;
}

QVariantMap ContentsCalloutBackend::buildCalloutInsertionPayload(const QString& bodyText) const
{
    QString escapedBodyText = escapeSourceLiteral(bodyText);
    const bool insertedEmptyAnchor = escapedBodyText.trimmed().isEmpty();
    if (insertedEmptyAnchor)
    {
        // Keep one editable anchor character so empty shortcut callouts remain cursor-reachable.
        escapedBodyText = QStringLiteral(" ");
    }
    const QString calloutOpenTag = QStringLiteral("<callout>");
    const QString calloutCloseTag = QStringLiteral("</callout>");
    const int cursorSourceOffsetFromInsertionStart = insertedEmptyAnchor
                                                         ? calloutOpenTag.size()
                                                         : calloutOpenTag.size() + escapedBodyText.size();

    QVariantMap insertionPayload;
    insertionPayload.insert(QStringLiteral("applied"), true);
    insertionPayload.insert(QStringLiteral("calloutOpenTag"), calloutOpenTag);
    insertionPayload.insert(QStringLiteral("calloutCloseTag"), calloutCloseTag);
    insertionPayload.insert(
        QStringLiteral("cursorSourceOffsetFromInsertionStart"),
        cursorSourceOffsetFromInsertionStart);
    insertionPayload.insert(
        QStringLiteral("insertionSourceText"),
        calloutOpenTag + escapedBodyText + calloutCloseTag);
    return insertionPayload;
}

QVariantMap ContentsCalloutBackend::detectCalloutEnterReplacement(
    const QString& sourceText,
    const int sourceStart,
    const int sourceEnd,
    const QString& insertedText) const
{
    const QString normalizedInsertedText = normalizePlainText(insertedText);
    if (!normalizedInsertedText.endsWith(QLatin1Char('\n')))
    {
        return notAppliedResult();
    }
    if (normalizedInsertedText.count(QLatin1Char('\n')) != 1)
    {
        return notAppliedResult();
    }

    const std::optional<CalloutContext> calloutContext = calloutContextAtSourceOffset(sourceText, sourceStart);
    if (!calloutContext.has_value())
    {
        return notAppliedResult();
    }

    const QString normalizedSourceText = sourceText;
    const int safeStart = boundedTextIndex(normalizedSourceText, sourceStart);
    const int safeEnd = boundedTextIndex(normalizedSourceText, std::max(safeStart, sourceEnd));
    if (safeStart != safeEnd)
    {
        return notAppliedResult();
    }

    const int lineStartCandidate =
        safeStart > calloutContext->calloutContentStart
        ? normalizedSourceText.lastIndexOf(QLatin1Char('\n'), safeStart - 1) + 1
        : calloutContext->calloutContentStart;
    const int lineStart = std::max(calloutContext->calloutContentStart, lineStartCandidate);
    if (lineStart <= calloutContext->calloutContentStart)
    {
        return notAppliedResult();
    }
    if (normalizedSourceText.at(lineStart - 1) != QLatin1Char('\n'))
    {
        return notAppliedResult();
    }

    const int lineEndIndex = normalizedSourceText.indexOf(QLatin1Char('\n'), safeEnd);
    const int lineEnd =
        lineEndIndex >= 0 && lineEndIndex < calloutContext->calloutCloseStart
        ? lineEndIndex
        : calloutContext->calloutCloseStart;
    const QString linePrefix = normalizedSourceText.mid(lineStart, safeStart - lineStart);
    const QString lineSuffix = normalizedSourceText.mid(safeEnd, lineEnd - safeEnd);
    if (!linePrefix.trimmed().isEmpty() || !lineSuffix.trimmed().isEmpty())
    {
        return notAppliedResult();
    }

    const QString trailingSourceText =
        normalizedSourceText.mid(safeEnd, calloutContext->calloutCloseStart - safeEnd);
    if (!trailingSourceText.trimmed().isEmpty())
    {
        return notAppliedResult();
    }

    const QString exitCalloutSourceText = QStringLiteral("</callout>\n");
    return {
        {QStringLiteral("applied"), true},
        {QStringLiteral("cursorSourceOffsetFromReplacementStart"), exitCalloutSourceText.size()},
        {QStringLiteral("replacementSourceStart"), lineStart - 1},
        {QStringLiteral("replacementSourceEnd"), calloutContext->calloutCloseEnd},
        {QStringLiteral("replacementSourceText"), exitCalloutSourceText}
    };
}
