#include "app/models/editor/tags/ContentsEditorTagInsertionController.hpp"

#include <algorithm>

#include <QDate>
#include <QRegularExpression>
#include <Qt>

namespace
{
    QString normalizedText(const QVariant& value)
    {
        QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar(0x2028), QLatin1Char('\n'));
        text.replace(QChar(0x2029), QLatin1Char('\n'));
        text.replace(QChar(0x00A0), QLatin1Char(' '));
        return text;
    }

    QString normalizeEditorTagName(const QVariant& rawTagName)
    {
        const QString tag = normalizedText(rawTagName).trimmed().toLower();
        if (tag == QLatin1String("plain") || tag == QLatin1String("clear") || tag == QLatin1String("none"))
            return QStringLiteral("plain");
        if (tag == QLatin1String("bold") || tag == QLatin1String("b") || tag == QLatin1String("strong"))
            return QStringLiteral("bold");
        if (tag == QLatin1String("italic") || tag == QLatin1String("i") || tag == QLatin1String("em"))
            return QStringLiteral("italic");
        if (tag == QLatin1String("underline") || tag == QLatin1String("u"))
            return QStringLiteral("underline");
        if (tag == QLatin1String("strikethrough") || tag == QLatin1String("strike") || tag == QLatin1String("s") || tag == QLatin1String("del"))
            return QStringLiteral("strikethrough");
        if (tag == QLatin1String("highlight") || tag == QLatin1String("mark"))
            return QStringLiteral("highlight");
        if (tag == QLatin1String("callout"))
            return QStringLiteral("callout");
        if (tag == QLatin1String("agenda"))
            return QStringLiteral("agenda");
        if (tag == QLatin1String("task"))
            return QStringLiteral("task");
        if (tag == QLatin1String("resource"))
            return QStringLiteral("resource");
        if (tag == QLatin1String("break") || tag == QLatin1String("hr"))
            return QStringLiteral("break");
        return {};
    }

    bool isGeneratedBodyInsertionTag(const QString& tagName)
    {
        return tagName == QLatin1String("agenda")
            || tagName == QLatin1String("callout")
            || tagName == QLatin1String("break");
    }

    bool isPairedTagInsertionTag(const QString& tagName)
    {
        return tagName == QLatin1String("bold")
            || tagName == QLatin1String("italic")
            || tagName == QLatin1String("underline")
            || tagName == QLatin1String("strikethrough")
            || tagName == QLatin1String("highlight")
            || tagName == QLatin1String("callout")
            || tagName == QLatin1String("agenda")
            || tagName == QLatin1String("task");
    }

    int boundedOffset(const int offset, const int sourceLength)
    {
        return std::clamp(offset, 0, std::max(0, sourceLength));
    }

    QString todayIsoDate()
    {
        return QDate::currentDate().toString(Qt::ISODate);
    }

    QString openingTagForName(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda"))
            return QStringLiteral("<agenda date=\"%1\"><task done=\"false\">").arg(todayIsoDate());
        return QStringLiteral("<%1>").arg(tagName);
    }

    QString closingTagForName(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda"))
            return QStringLiteral("</task></agenda>");
        return QStringLiteral("</%1>").arg(tagName);
    }

    QString canonicalSourceForGeneratedBodyTag(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda"))
            return openingTagForName(tagName) + QLatin1Char(' ') + closingTagForName(tagName);
        if (tagName == QLatin1String("callout"))
            return QStringLiteral("<callout> </callout>");
        if (tagName == QLatin1String("break"))
            return QStringLiteral("</break>");
        return {};
    }

    int cursorOffsetForGeneratedBodyTag(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda") || tagName == QLatin1String("callout"))
            return openingTagForName(tagName).length();
        if (tagName == QLatin1String("break"))
            return canonicalSourceForGeneratedBodyTag(tagName).length();
        return 0;
    }

    bool sourceRangeOverlapsStructuredBodyBlock(const QString& sourceText, const int selectionStart, const int selectionEnd)
    {
        static const QRegularExpression structuredBodyBlockPattern(
            QStringLiteral(R"(<(?:agenda|callout)\b[^>]*>[\s\S]*?</(?:agenda|callout)>)"),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator iterator = structuredBodyBlockPattern.globalMatch(sourceText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
                continue;
            const int blockStart = std::max(0, static_cast<int>(match.capturedStart(0)));
            const int blockEnd = std::max(blockStart, static_cast<int>(match.capturedEnd(0)));
            if (selectionStart < blockEnd && selectionEnd > blockStart)
                return true;
        }
        return false;
    }

    bool sourceTextContainsDocumentBlockTagToken(const QString& sourceText)
    {
        static const QRegularExpression documentBlockTagPattern(
            QStringLiteral(R"(<\s*/?\s*(?:agenda|task|callout|resource|break|hr)\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);
        return sourceText.contains(documentBlockTagPattern);
    }

    int resolveStructuredBodyTagInsertionOffset(const QString& sourceText, const int requestedInsertionOffset)
    {
        const int safeRequestedOffset = boundedOffset(requestedInsertionOffset, sourceText.length());
        static const QRegularExpression structuredBodyBlockPattern(
            QStringLiteral(R"(<(?:agenda|callout)\b[^>]*>[\s\S]*?</(?:agenda|callout)>)"),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator iterator = structuredBodyBlockPattern.globalMatch(sourceText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
                continue;
            const int blockStart = std::max(0, static_cast<int>(match.capturedStart(0)));
            const int blockEnd = std::max(blockStart, static_cast<int>(match.capturedEnd(0)));
            if (safeRequestedOffset > blockStart && safeRequestedOffset < blockEnd)
                return blockEnd;
        }
        return safeRequestedOffset;
    }

    QVariantMap notAppliedPayload(
        const QString& reason,
        const QString& sourceText,
        const int selectionStart,
        const int selectionEnd,
        const QString& tagName)
    {
        const int boundedStart = boundedOffset(std::min(selectionStart, selectionEnd), sourceText.length());
        const int boundedEnd = boundedOffset(std::max(selectionStart, selectionEnd), sourceText.length());
        return QVariantMap{
            {QStringLiteral("applied"), false},
            {QStringLiteral("cursorPosition"), boundedEnd},
            {QStringLiteral("nextSourceText"), sourceText},
            {QStringLiteral("reason"), reason},
            {QStringLiteral("selectionEnd"), boundedEnd},
            {QStringLiteral("selectionStart"), boundedStart},
            {QStringLiteral("tagName"), tagName},
        };
    }

    QVariantMap buildGeneratedBodyTagInsertionPayload(
        const QString& source,
        const int requestedInsertionOffset,
        const QString& tagName)
    {
        const QString rawSourceText = canonicalSourceForGeneratedBodyTag(tagName);
        if (rawSourceText.isEmpty())
            return notAppliedPayload(QStringLiteral("unsupported-generated-tag"), source, requestedInsertionOffset, requestedInsertionOffset, tagName);

        const int requestedOffset = boundedOffset(requestedInsertionOffset, source.length());
        const int insertionOffset = resolveStructuredBodyTagInsertionOffset(source, requestedOffset);
        const QString prefixNewline =
            insertionOffset > 0 && source.at(insertionOffset - 1) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString suffixNewline =
            insertionOffset < source.length() && source.at(insertionOffset) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString insertedSourceText = prefixNewline + rawSourceText + suffixNewline;
        const QString nextSourceText = source.left(insertionOffset) + insertedSourceText + source.mid(insertionOffset);
        const int cursorPosition = insertionOffset
            + prefixNewline.length()
            + boundedOffset(cursorOffsetForGeneratedBodyTag(tagName), rawSourceText.length());

        return QVariantMap{
            {QStringLiteral("applied"), nextSourceText != source},
            {QStringLiteral("cursorPosition"), cursorPosition},
            {QStringLiteral("insertedSourceText"), insertedSourceText},
            {QStringLiteral("nextSourceText"), nextSourceText},
            {QStringLiteral("rawSourceText"), rawSourceText},
            {QStringLiteral("reason"), nextSourceText != source ? QString{} : QStringLiteral("no-op")},
            {QStringLiteral("requestedInsertionOffset"), requestedOffset},
            {QStringLiteral("resolvedInsertionOffset"), insertionOffset},
            {QStringLiteral("selectionEnd"), cursorPosition},
            {QStringLiteral("selectionStart"), cursorPosition},
            {QStringLiteral("sourceOffset"), cursorPosition},
            {QStringLiteral("tagKind"), tagName},
            {QStringLiteral("tagName"), tagName},
        };
    }
} // namespace

ContentsEditorTagInsertionController::ContentsEditorTagInsertionController(QObject* parent)
    : QObject(parent)
{
}

QString ContentsEditorTagInsertionController::normalizedTagName(const QVariant& tagName) const
{
    return normalizeEditorTagName(tagName);
}

QString ContentsEditorTagInsertionController::tagNameForBodyShortcutKey(const int key) const
{
    switch (key)
    {
    case Qt::Key_A:
        return QStringLiteral("agenda");
    case Qt::Key_C:
        return QStringLiteral("callout");
    case Qt::Key_B:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return QStringLiteral("break");
    default:
        return {};
    }
}

QString ContentsEditorTagInsertionController::tagNameForShortcutKey(const int key) const
{
    switch (key)
    {
    case Qt::Key_B:
        return QStringLiteral("bold");
    case Qt::Key_I:
        return QStringLiteral("italic");
    case Qt::Key_U:
        return QStringLiteral("underline");
    case Qt::Key_H:
        return QStringLiteral("highlight");
    default:
        return {};
    }
}

QVariantMap ContentsEditorTagInsertionController::buildTagInsertionPayload(
    const QVariant& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QVariant& tagName)
{
    const QString source = normalizedText(sourceText);
    const QString normalizedTag = normalizeEditorTagName(tagName);
    if (normalizedTag.isEmpty())
        return notAppliedPayload(QStringLiteral("unsupported-tag-kind"), source, selectionStart, selectionEnd, normalizedTag);
    if (selectionStart == selectionEnd && isGeneratedBodyInsertionTag(normalizedTag))
    {
        QVariantMap payload = buildGeneratedBodyTagInsertionPayload(source, selectionStart, normalizedTag);
        emit tagInsertionPayloadBuilt(payload);
        return payload;
    }
    return buildWrappedTagInsertionPayload(source, selectionStart, selectionEnd, normalizedTag);
}

QVariantMap ContentsEditorTagInsertionController::buildWrappedTagInsertionPayload(
    const QVariant& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QVariant& tagName)
{
    const QString source = normalizedText(sourceText);
    const QString normalizedTag = normalizeEditorTagName(tagName);
    if (normalizedTag.isEmpty())
        return notAppliedPayload(QStringLiteral("unsupported-tag-kind"), source, selectionStart, selectionEnd, normalizedTag);
    if (normalizedTag == QLatin1String("plain"))
        return notAppliedPayload(QStringLiteral("plain-tag-not-wrappable"), source, selectionStart, selectionEnd, normalizedTag);
    if (!isPairedTagInsertionTag(normalizedTag))
        return notAppliedPayload(QStringLiteral("tag-not-paired-wrappable"), source, selectionStart, selectionEnd, normalizedTag);

    const int sourceLength = source.length();
    const int start = boundedOffset(std::min(selectionStart, selectionEnd), sourceLength);
    const int end = boundedOffset(std::max(selectionStart, selectionEnd), sourceLength);
    if (end <= start && isGeneratedBodyInsertionTag(normalizedTag))
    {
        QVariantMap payload = buildGeneratedBodyTagInsertionPayload(source, start, normalizedTag);
        emit tagInsertionPayloadBuilt(payload);
        return payload;
    }

    const bool bodyBlockWrap = normalizedTag == QLatin1String("agenda")
        || normalizedTag == QLatin1String("callout");
    if (bodyBlockWrap)
    {
        if (sourceRangeOverlapsStructuredBodyBlock(source, start, end))
            return notAppliedPayload(QStringLiteral("body-tag-range-overlaps-structured-block"), source, selectionStart, selectionEnd, normalizedTag);
        if (sourceTextContainsDocumentBlockTagToken(source.mid(start, end - start)))
            return notAppliedPayload(QStringLiteral("body-tag-range-contains-document-block-tag"), source, selectionStart, selectionEnd, normalizedTag);
    }

    const QString openingTag = openingTagForName(normalizedTag);
    const QString closingTag = closingTagForName(normalizedTag);

    QVariantMap payload;
    if (end <= start)
    {
        const QString nextSource = source.left(start) + openingTag + closingTag + source.mid(start);
        const int cursorPosition = start + openingTag.length();
        payload = {
            {QStringLiteral("applied"), nextSource != source},
            {QStringLiteral("cursorPosition"), cursorPosition},
            {QStringLiteral("insertedSourceText"), openingTag + closingTag},
            {QStringLiteral("nextSourceText"), nextSource},
            {QStringLiteral("reason"), nextSource != source ? QString{} : QStringLiteral("no-op")},
            {QStringLiteral("selectionEnd"), cursorPosition},
            {QStringLiteral("selectionStart"), cursorPosition},
            {QStringLiteral("tagName"), normalizedTag},
        };
    }
    else
    {
        const QString selectedSource = source.mid(start, end - start);
        const QString prefixNewline =
            bodyBlockWrap && start > 0 && source.at(start - 1) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString suffixNewline =
            bodyBlockWrap && end < sourceLength && source.at(end) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString replacementSourceText = openingTag + selectedSource + closingTag;
        const QString fullReplacementSourceText = prefixNewline + replacementSourceText + suffixNewline;
        const QString nextSource = source.left(start) + fullReplacementSourceText + source.mid(end);
        const int wrappedSelectionStart = start + prefixNewline.length() + openingTag.length();
        const int wrappedSelectionEnd = wrappedSelectionStart + selectedSource.length();
        payload = {
            {QStringLiteral("applied"), nextSource != source},
            {QStringLiteral("cursorPosition"), wrappedSelectionEnd},
            {QStringLiteral("insertedSourceText"), openingTag + closingTag},
            {QStringLiteral("nextSourceText"), nextSource},
            {QStringLiteral("reason"), nextSource != source ? QString{} : QStringLiteral("no-op")},
            {QStringLiteral("replacementSourceText"), fullReplacementSourceText},
            {QStringLiteral("selectionEnd"), wrappedSelectionEnd},
            {QStringLiteral("selectionStart"), wrappedSelectionStart},
            {QStringLiteral("tagKind"), normalizedTag},
            {QStringLiteral("tagName"), normalizedTag},
            {QStringLiteral("wrappedSourceText"), selectedSource},
        };
    }

    emit tagInsertionPayloadBuilt(payload);
    return payload;
}
