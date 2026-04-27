#include "app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.hpp"

#include "app/models/editor/tags/ContentsAgendaBackend.hpp"
#include "app/models/editor/tags/ContentsCalloutBackend.hpp"

#include <QRegularExpression>

#include <algorithm>

namespace
{
    QString normalizedShortcutKind(const QString& shortcutKind)
    {
        return shortcutKind.trimmed().toLower();
    }

    int boundedOffset(const QString& sourceText, const int offset)
    {
        return std::clamp(offset, 0, static_cast<int>(sourceText.size()));
    }

    int floorOffset(const int offset)
    {
        return std::max(0, offset);
    }

    bool canonicalAgendaSourceText(const QString& sourceText)
    {
        static const QRegularExpression pattern(
            QStringLiteral(R"(^<agenda\b[^>]*>\s*<task\b[^>]*>[\s\S]*</task>\s*</agenda>$)"),
            QRegularExpression::CaseInsensitiveOption);
        return pattern.match(sourceText).hasMatch();
    }

    bool canonicalCalloutSourceText(const QString& sourceText)
    {
        static const QRegularExpression pattern(
            QStringLiteral(R"(^<callout>[\s\S]*</callout>$)"),
            QRegularExpression::CaseInsensitiveOption);
        return pattern.match(sourceText).hasMatch();
    }

    bool sourceRangeOverlapsStructuredBlock(
        const QString& sourceText,
        const int selectionStart,
        const int selectionEnd)
    {
        static const QRegularExpression structuredBlockPattern(
            QStringLiteral(R"(<(?:agenda|callout)\b[^>]*>[\s\S]*?</(?:agenda|callout)>)"),
            QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator matches = structuredBlockPattern.globalMatch(sourceText);
        while (matches.hasNext())
        {
            const QRegularExpressionMatch match = matches.next();
            const int blockStart = std::max(0, static_cast<int>(match.capturedStart(0)));
            const int blockEnd = std::max(blockStart, static_cast<int>(match.capturedEnd(0)));
            if (selectionStart < blockEnd && selectionEnd > blockStart)
            {
                return true;
            }
        }

        return false;
    }

    bool sourceTextContainsDocumentBlockTagToken(const QString& sourceText)
    {
        static const QRegularExpression documentBlockTagPattern(
            QStringLiteral(R"(<\s*/?\s*(?:agenda|task|callout|resource|break|hr)\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);
        return documentBlockTagPattern.match(sourceText).hasMatch();
    }

    QVariantMap normalizedSpecPayload(
        const QString& tagKind,
        const QString& insertionSourceText,
        const int cursorSourceOffsetFromInsertionStart)
    {
        QVariantMap payload;
        payload.insert(QStringLiteral("applied"), true);
        payload.insert(QStringLiteral("tagKind"), tagKind);
        payload.insert(
            QStringLiteral("cursorSourceOffsetFromInsertionStart"),
            std::max(0, cursorSourceOffsetFromInsertionStart));
        payload.insert(QStringLiteral("insertionSourceText"), insertionSourceText);
        return payload;
    }
} // namespace

ContentsEditorBodyTagInsertionPlanner::ContentsEditorBodyTagInsertionPlanner(QObject* parent)
    : QObject(parent)
{
}

ContentsEditorBodyTagInsertionPlanner::~ContentsEditorBodyTagInsertionPlanner() = default;

QObject* ContentsEditorBodyTagInsertionPlanner::agendaBackend() const noexcept
{
    return m_agendaBackend;
}

void ContentsEditorBodyTagInsertionPlanner::setAgendaBackend(QObject* agendaBackend)
{
    auto* const nextAgendaBackend = qobject_cast<ContentsAgendaBackend*>(agendaBackend);
    if (m_agendaBackend == nextAgendaBackend)
    {
        return;
    }

    m_agendaBackend = nextAgendaBackend;
    emit agendaBackendChanged();
}

QObject* ContentsEditorBodyTagInsertionPlanner::calloutBackend() const noexcept
{
    return m_calloutBackend;
}

void ContentsEditorBodyTagInsertionPlanner::setCalloutBackend(QObject* calloutBackend)
{
    auto* const nextCalloutBackend = qobject_cast<ContentsCalloutBackend*>(calloutBackend);
    if (m_calloutBackend == nextCalloutBackend)
    {
        return;
    }

    m_calloutBackend = nextCalloutBackend;
    emit calloutBackendChanged();
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::structuredShortcutInsertionSpec(
    const QString& shortcutKind) const
{
    const QString tagKind = normalizedShortcutKind(shortcutKind);
    if (tagKind == QStringLiteral("agenda"))
    {
        return agendaInsertionSpec();
    }

    if (tagKind == QStringLiteral("callout"))
    {
        return calloutInsertionSpec();
    }

    if (tagKind == QStringLiteral("break"))
    {
        return breakInsertionSpec();
    }

    return notAppliedPayload(QStringLiteral("unsupported-tag-kind"));
}

int ContentsEditorBodyTagInsertionPlanner::resolveStructuredTagInsertionOffset(
    const QString& sourceText,
    const int requestedInsertionOffset) const
{
    const QString normalizedSource = normalizeSourceText(sourceText);
    const int safeRequestedOffset = boundedOffset(normalizedSource, requestedInsertionOffset);
    static const QRegularExpression structuredBlockPattern(
        QStringLiteral(R"(<(?:agenda|callout)\b[^>]*>[\s\S]*?</(?:agenda|callout)>)"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator matches = structuredBlockPattern.globalMatch(normalizedSource);
    while (matches.hasNext())
    {
        const QRegularExpressionMatch match = matches.next();
        const int blockStart = std::max(0, static_cast<int>(match.capturedStart(0)));
        const int blockEnd = std::max(blockStart, static_cast<int>(match.capturedEnd(0)));
        if (safeRequestedOffset > blockStart && safeRequestedOffset < blockEnd)
        {
            return boundedOffset(normalizedSource, blockEnd);
        }
    }

    return safeRequestedOffset;
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::buildRawSourceInsertionPayload(
    const QString& sourceText,
    const int requestedInsertionOffset,
    const QString& rawSourceText,
    const int cursorSourceOffsetFromInsertionStart) const
{
    const QString currentSourceText = normalizeSourceText(sourceText);
    const QString normalizedRawSourceText = normalizeSourceText(rawSourceText).trimmed();
    if (normalizedRawSourceText.isEmpty())
    {
        return notAppliedPayload(QStringLiteral("empty-tag-source"));
    }

    const int insertionOffset = resolveStructuredTagInsertionOffset(
        currentSourceText,
        requestedInsertionOffset);
    const QString prefixNewline =
        insertionOffset > 0 && currentSourceText.at(insertionOffset - 1) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString suffixNewline =
        insertionOffset < static_cast<int>(currentSourceText.size())
            && currentSourceText.at(insertionOffset) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString insertedSourceText = prefixNewline + normalizedRawSourceText + suffixNewline;
    const int cursorOffsetInsideRawSource = std::clamp(
        cursorSourceOffsetFromInsertionStart,
        0,
        static_cast<int>(normalizedRawSourceText.size()));
    const int sourceOffset =
        insertionOffset + static_cast<int>(prefixNewline.size()) + cursorOffsetInsideRawSource;
    const QString nextSourceText = spliceSourceText(
        currentSourceText,
        insertionOffset,
        insertionOffset,
        insertedSourceText);

    QVariantMap payload;
    payload.insert(QStringLiteral("applied"), nextSourceText != currentSourceText);
    payload.insert(QStringLiteral("nextSourceText"), nextSourceText);
    payload.insert(QStringLiteral("sourceOffset"), sourceOffset);
    payload.insert(QStringLiteral("insertedSourceText"), insertedSourceText);
    payload.insert(QStringLiteral("rawSourceText"), normalizedRawSourceText);
    payload.insert(QStringLiteral("resolvedInsertionOffset"), insertionOffset);
    payload.insert(QStringLiteral("requestedInsertionOffset"), boundedOffset(currentSourceText, requestedInsertionOffset));
    return payload;
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::buildStructuredShortcutInsertionPayload(
    const QString& sourceText,
    const int requestedInsertionOffset,
    const QString& shortcutKind) const
{
    const QVariantMap insertionSpec = structuredShortcutInsertionSpec(shortcutKind);
    if (!insertionSpec.value(QStringLiteral("applied")).toBool())
    {
        return insertionSpec;
    }

    QVariantMap payload = buildRawSourceInsertionPayload(
        sourceText,
        requestedInsertionOffset,
        insertionSpec.value(QStringLiteral("insertionSourceText")).toString(),
        insertionSpec.value(QStringLiteral("cursorSourceOffsetFromInsertionStart")).toInt());
    payload.insert(QStringLiteral("tagKind"), insertionSpec.value(QStringLiteral("tagKind")));
    payload.insert(
        QStringLiteral("cursorSourceOffsetFromInsertionStart"),
        insertionSpec.value(QStringLiteral("cursorSourceOffsetFromInsertionStart")).toInt());
    return payload;
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::buildCalloutRangeWrappingPayload(
    const QString& sourceText,
    const int selectionSourceStart,
    const int selectionSourceEnd) const
{
    const QString currentSourceText = normalizeSourceText(sourceText);
    const int sourceLength = static_cast<int>(currentSourceText.size());
    const int rangeStart = std::clamp(
        std::min(selectionSourceStart, selectionSourceEnd),
        0,
        sourceLength);
    const int rangeEnd = std::clamp(
        std::max(selectionSourceStart, selectionSourceEnd),
        rangeStart,
        sourceLength);
    if (rangeEnd <= rangeStart)
    {
        return notAppliedPayload(QStringLiteral("empty-callout-range"));
    }
    if (sourceRangeOverlapsStructuredBlock(currentSourceText, rangeStart, rangeEnd))
    {
        return notAppliedPayload(QStringLiteral("callout-range-overlaps-structured-block"));
    }

    const QString selectedSourceText = currentSourceText.mid(rangeStart, rangeEnd - rangeStart);
    if (sourceTextContainsDocumentBlockTagToken(selectedSourceText))
    {
        return notAppliedPayload(QStringLiteral("callout-range-contains-document-block-tag"));
    }

    const QString calloutOpenTag = QStringLiteral("<callout>");
    const QString calloutCloseTag = QStringLiteral("</callout>");
    const QString wrappedCalloutSourceText = calloutOpenTag + selectedSourceText + calloutCloseTag;
    if (!canonicalCalloutSourceText(wrappedCalloutSourceText))
    {
        return notAppliedPayload(QStringLiteral("invalid-callout-source"));
    }

    const QString prefixNewline =
        rangeStart > 0 && currentSourceText.at(rangeStart - 1) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString suffixNewline =
        rangeEnd < sourceLength && currentSourceText.at(rangeEnd) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString replacementSourceText = prefixNewline + wrappedCalloutSourceText + suffixNewline;
    const QString nextSourceText = spliceSourceText(
        currentSourceText,
        rangeStart,
        rangeEnd,
        replacementSourceText);
    const int sourceOffset =
        rangeStart
        + static_cast<int>(prefixNewline.size())
        + static_cast<int>(calloutOpenTag.size())
        + static_cast<int>(selectedSourceText.size());

    QVariantMap payload;
    payload.insert(QStringLiteral("applied"), nextSourceText != currentSourceText);
    payload.insert(QStringLiteral("tagKind"), QStringLiteral("callout"));
    payload.insert(QStringLiteral("nextSourceText"), nextSourceText);
    payload.insert(QStringLiteral("sourceOffset"), sourceOffset);
    payload.insert(QStringLiteral("replacementSourceText"), replacementSourceText);
    payload.insert(QStringLiteral("wrappedSourceText"), selectedSourceText);
    payload.insert(QStringLiteral("resolvedRangeStart"), rangeStart);
    payload.insert(QStringLiteral("resolvedRangeEnd"), rangeEnd);
    payload.insert(QStringLiteral("requestedRangeStart"), floorOffset(selectionSourceStart));
    payload.insert(QStringLiteral("requestedRangeEnd"), floorOffset(selectionSourceEnd));
    return payload;
}

QString ContentsEditorBodyTagInsertionPlanner::normalizeSourceText(const QString& sourceText)
{
    QString normalizedText = sourceText;
    normalizedText.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalizedText.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    normalizedText.replace(QChar(0x2028), QLatin1Char('\n'));
    normalizedText.replace(QChar(0x2029), QLatin1Char('\n'));
    normalizedText.replace(QChar(0xfffc), QString());
    normalizedText.replace(QChar(0x00a0), QLatin1Char(' '));
    return normalizedText;
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::notAppliedPayload(const QString& reason)
{
    QVariantMap payload;
    payload.insert(QStringLiteral("applied"), false);
    payload.insert(QStringLiteral("reason"), reason);
    return payload;
}

QString ContentsEditorBodyTagInsertionPlanner::spliceSourceText(
    const QString& sourceText,
    const int sourceStart,
    const int sourceEnd,
    const QString& replacementSourceText)
{
    const int safeLength = static_cast<int>(sourceText.size());
    const int boundedStart = std::clamp(sourceStart, 0, safeLength);
    const int boundedEnd = std::clamp(sourceEnd, boundedStart, safeLength);
    return sourceText.left(boundedStart) + replacementSourceText + sourceText.mid(boundedEnd);
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::agendaInsertionSpec() const
{
    if (m_agendaBackend == nullptr)
    {
        return notAppliedPayload(QStringLiteral("missing-agenda-backend"));
    }

    const QVariantMap insertionPayload = m_agendaBackend->buildAgendaInsertionPayload(false, QString());
    const QString insertionSourceText = insertionPayload.value(QStringLiteral("insertionSourceText")).toString();
    if (!canonicalAgendaSourceText(insertionSourceText))
    {
        return notAppliedPayload(QStringLiteral("invalid-agenda-source"));
    }

    return normalizedSpecPayload(
        QStringLiteral("agenda"),
        insertionSourceText,
        insertionPayload.value(QStringLiteral("cursorSourceOffsetFromInsertionStart")).toInt());
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::calloutInsertionSpec() const
{
    if (m_calloutBackend == nullptr)
    {
        return notAppliedPayload(QStringLiteral("missing-callout-backend"));
    }

    const QVariantMap insertionPayload = m_calloutBackend->buildCalloutInsertionPayload(QString());
    const QString insertionSourceText = insertionPayload.value(QStringLiteral("insertionSourceText")).toString();
    if (!canonicalCalloutSourceText(insertionSourceText))
    {
        return notAppliedPayload(QStringLiteral("invalid-callout-source"));
    }

    return normalizedSpecPayload(
        QStringLiteral("callout"),
        insertionSourceText,
        insertionPayload.value(QStringLiteral("cursorSourceOffsetFromInsertionStart")).toInt());
}

QVariantMap ContentsEditorBodyTagInsertionPlanner::breakInsertionSpec() const
{
    return normalizedSpecPayload(
        QStringLiteral("break"),
        QStringLiteral("</break>"),
        QStringLiteral("</break>").size());
}
