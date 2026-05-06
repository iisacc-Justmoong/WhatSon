#include "app/models/editor/text/ContentsLogicalTextBridge.hpp"

#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QChar>
#include <QStringList>
#include <QStringView>
#include <algorithm>
#include <limits>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    constexpr char16_t kResourceLogicalPlaceholderCharacter = 0xfffc;
    constexpr int kResourceLogicalPlaceholderLength = 1;

    int boundedContainerSize(const qsizetype size) noexcept
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        return static_cast<int>(std::clamp(size, qsizetype{0}, maxIntSize));
    }

    int boundedQStringSize(const QString& text) noexcept
    {
        return boundedContainerSize(text.size());
    }

    QString normalizedHtmlTagName(const QStringView tagToken)
    {
        if (tagToken.size() < 3 || tagToken.front() != QLatin1Char('<'))
        {
            return {};
        }

        int cursor = 1;
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
        {
            ++cursor;
        }
        if (cursor < tagToken.size() && tagToken.at(cursor) == QLatin1Char('/'))
        {
            ++cursor;
        }
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
        {
            ++cursor;
        }

        const int nameStart = cursor;
        while (cursor < tagToken.size())
        {
            const QChar ch = tagToken.at(cursor);
            if (!(ch.isLetterOrNumber()
                  || ch == QLatin1Char('_')
                  || ch == QLatin1Char('.')
                  || ch == QLatin1Char(':')
                  || ch == QLatin1Char('-')))
            {
                break;
            }
            ++cursor;
        }

        if (cursor <= nameStart)
        {
            return {};
        }

        return tagToken.mid(nameStart, cursor - nameStart).toString().trimmed().toCaseFolded();
    }

    bool isClosingHtmlTagToken(const QStringView tagToken)
    {
        if (tagToken.size() < 3 || tagToken.front() != QLatin1Char('<'))
        {
            return false;
        }

        int cursor = 1;
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
        {
            ++cursor;
        }

        return cursor < tagToken.size() && tagToken.at(cursor) == QLatin1Char('/');
    }

    bool htmlTagProducesLogicalBreak(const QStringView tagToken)
    {
        const QString normalizedTagName = normalizedHtmlTagName(tagToken);
        return normalizedTagName == QStringLiteral("br")
            || normalizedTagName == QStringLiteral("break")
            || normalizedTagName == QStringLiteral("hr");
    }

    bool advancesCursorPastClosingInlineBoundaryTag(const QString& tagName)
    {
        return tagName == QStringLiteral("bold")
            || tagName == QStringLiteral("b")
            || tagName == QStringLiteral("strong")
            || tagName == QStringLiteral("italic")
            || tagName == QStringLiteral("i")
            || tagName == QStringLiteral("em")
            || tagName == QStringLiteral("underline")
            || tagName == QStringLiteral("u")
            || tagName == QStringLiteral("strikethrough")
            || tagName == QStringLiteral("strike")
            || tagName == QStringLiteral("s")
            || tagName == QStringLiteral("del")
            || tagName == QStringLiteral("highlight")
            || tagName == QStringLiteral("mark")
            || SemanticTags::isWebLinkTagName(tagName);
    }

    int advanceSourceOffsetPastClosingInlineBoundaryTags(const QString& text, int sourceOffset)
    {
        int boundedOffset = std::clamp(sourceOffset, 0, boundedQStringSize(text));
        while (boundedOffset < text.size() && text.at(boundedOffset) == QLatin1Char('<'))
        {
            const int tagEnd = text.indexOf(QLatin1Char('>'), boundedOffset + 1);
            if (tagEnd <= boundedOffset)
            {
                break;
            }

            const QStringView tagToken(text.constData() + boundedOffset, tagEnd - boundedOffset + 1);
            const QString normalizedTagName = normalizedHtmlTagName(tagToken);
            if (!isClosingHtmlTagToken(tagToken)
                || !advancesCursorPastClosingInlineBoundaryTag(normalizedTagName))
            {
                break;
            }

            boundedOffset = tagEnd + 1;
        }
        return boundedOffset;
    }

    int htmlEntityLengthAt(const QString& text, const int sourceOffset)
    {
        if (sourceOffset < 0
            || sourceOffset >= text.size()
            || text.at(sourceOffset) != QLatin1Char('&'))
        {
            return 0;
        }

        const int semicolonOffset = text.indexOf(QLatin1Char(';'), sourceOffset + 1);
        if (semicolonOffset <= sourceOffset)
        {
            return 0;
        }

        const QString entityToken = text.mid(sourceOffset, semicolonOffset - sourceOffset + 1).toCaseFolded();
        if (entityToken == QStringLiteral("&amp;")
            || entityToken == QStringLiteral("&lt;")
            || entityToken == QStringLiteral("&gt;")
            || entityToken == QStringLiteral("&quot;")
            || entityToken == QStringLiteral("&apos;")
            || entityToken == QStringLiteral("&#39;")
            || entityToken == QStringLiteral("&nbsp;"))
        {
            return entityToken.size();
        }

        if (entityToken.startsWith(QStringLiteral("&#x"))
            || entityToken.startsWith(QStringLiteral("&#")))
        {
            return entityToken.size();
        }

        return 0;
    }

    QString normalizeLineEndings(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return text;
    }

    QString resourceLogicalPlaceholderText()
    {
        return QString(1, QChar(kResourceLogicalPlaceholderCharacter));
    }

    QString decodedHtmlEntityText(const QStringView entityToken)
    {
        const QString normalizedToken = entityToken.toString().toCaseFolded();
        if (normalizedToken == QStringLiteral("&amp;"))
        {
            return QStringLiteral("&");
        }
        if (normalizedToken == QStringLiteral("&lt;"))
        {
            return QStringLiteral("<");
        }
        if (normalizedToken == QStringLiteral("&gt;"))
        {
            return QStringLiteral(">");
        }
        if (normalizedToken == QStringLiteral("&quot;"))
        {
            return QStringLiteral("\"");
        }
        if (normalizedToken == QStringLiteral("&apos;")
            || normalizedToken == QStringLiteral("&#39;"))
        {
            return QStringLiteral("'");
        }
        if (normalizedToken == QStringLiteral("&nbsp;"))
        {
            return QStringLiteral(" ");
        }

        bool ok = false;
        uint codePoint = 0;
        if (normalizedToken.startsWith(QStringLiteral("&#x")))
        {
            codePoint = normalizedToken.mid(3, normalizedToken.size() - 4).toUInt(&ok, 16);
        }
        else if (normalizedToken.startsWith(QStringLiteral("&#")))
        {
            codePoint = normalizedToken.mid(2, normalizedToken.size() - 3).toUInt(&ok, 10);
        }

        if (!ok || codePoint == 0 || codePoint > 0x10FFFFU)
        {
            return entityToken.toString();
        }

        if (!QChar::requiresSurrogates(codePoint))
        {
            QString decoded;
            decoded += QChar(static_cast<char16_t>(codePoint));
            return decoded;
        }

        QString decoded;
        decoded += QChar::highSurrogate(codePoint);
        decoded += QChar::lowSurrogate(codePoint);
        return decoded;
    }

} // namespace

ContentsLogicalTextBridge::ContentsLogicalTextBridge(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("logicalTextBridge"), QStringLiteral("ctor"));
    refreshTextState();
}

ContentsLogicalTextBridge::~ContentsLogicalTextBridge()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("logicalTextBridge"),
        QStringLiteral("dtor"),
        QStringLiteral("lineCount=%1 %2")
            .arg(m_logicalLineCount)
            .arg(WhatSon::Debug::summarizeText(m_text)));
}

QString ContentsLogicalTextBridge::text() const
{
    return m_text;
}

QString ContentsLogicalTextBridge::logicalText() const
{
    return m_logicalText;
}

void ContentsLogicalTextBridge::setText(const QString& text)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("logicalTextBridge"),
        QStringLiteral("setText"),
        QStringLiteral("changed=%1 %2").arg(m_text != text).arg(WhatSon::Debug::summarizeText(text)));
    if (m_text == text)
    {
        return;
    }

    m_text = text;
    emit textChanged();
    refreshTextState();
}

int ContentsLogicalTextBridge::logicalLineCount() const noexcept
{
    return m_logicalLineCount;
}

int ContentsLogicalTextBridge::logicalLengthForSourceText(const QString& text) const
{
    return boundedQStringSize(normalizeLogicalText(text));
}

QVariantList ContentsLogicalTextBridge::logicalToSourceOffsets() const
{
    QVariantList offsets;
    offsets.reserve(boundedContainerSize(m_logicalToSourceOffsets.size()));
    for (const int offset : m_logicalToSourceOffsets)
    {
        offsets.push_back(offset);
    }
    return offsets;
}

int ContentsLogicalTextBridge::sourceOffsetForLogicalOffset(int logicalOffset) const noexcept
{
    const int maxSourceOffset = boundedQStringSize(m_text);
    if (m_logicalToSourceOffsets.isEmpty())
    {
        return std::clamp(logicalOffset, 0, maxSourceOffset);
    }

    const int safeOffset = std::clamp(logicalOffset, 0, static_cast<int>(m_logicalToSourceOffsets.size()) - 1);
    return std::clamp(m_logicalToSourceOffsets.at(safeOffset), 0, maxSourceOffset);
}

int ContentsLogicalTextBridge::sourceOffsetForVisibleLogicalOffset(
    const int logicalOffset,
    const int visibleLength) const noexcept
{
    const int safeLogicalOffset = std::clamp(logicalOffset, 0, std::max(0, visibleLength));
    return sourceOffsetForLogicalOffset(safeLogicalOffset);
}

int ContentsLogicalTextBridge::logicalOffsetForSourceOffset(const int sourceOffset) const
{
    return logicalOffsetForSourceOffsetInText(m_text, sourceOffset);
}

int ContentsLogicalTextBridge::logicalOffsetForSourceOffsetWithAffinity(
    const int sourceOffset,
    const bool preferAfter) const noexcept
{
    const int maxSourceOffset = boundedQStringSize(m_text);
    if (m_logicalToSourceOffsets.isEmpty())
    {
        return std::clamp(sourceOffset, 0, maxSourceOffset);
    }

    const int boundedSourceOffset = std::clamp(sourceOffset, 0, maxSourceOffset);
    if (preferAfter)
    {
        for (int index = 0; index < m_logicalToSourceOffsets.size(); ++index)
        {
            if (m_logicalToSourceOffsets.at(index) >= boundedSourceOffset)
            {
                return index;
            }
        }
        return std::max(0, static_cast<int>(m_logicalToSourceOffsets.size()) - 1);
    }

    for (int index = static_cast<int>(m_logicalToSourceOffsets.size()) - 1; index >= 0; --index)
    {
        if (m_logicalToSourceOffsets.at(index) <= boundedSourceOffset)
        {
            return index;
        }
    }
    return 0;
}

QString ContentsLogicalTextBridge::normalizeLogicalText(const QString& text)
{
    const QString normalizedText = normalizeLineEndings(text);
    QString logicalText;
    logicalText.reserve(normalizedText.size());

    bool insideAgenda = false;
    int agendaTaskCount = 0;
    int sourceOffset = 0;
    while (sourceOffset < normalizedText.size())
    {
        const QChar ch = normalizedText.at(sourceOffset);
        if (ch == QLatin1Char('<'))
        {
            const int tagEnd = normalizedText.indexOf(QLatin1Char('>'), sourceOffset + 1);
            if (tagEnd > sourceOffset)
            {
                const QStringView tagToken(normalizedText.constData() + sourceOffset, tagEnd - sourceOffset + 1);
                const QString normalizedTagName = normalizedHtmlTagName(tagToken);
                const bool closingTag = isClosingHtmlTagToken(tagToken);

                if (normalizedTagName == QStringLiteral("agenda"))
                {
                    insideAgenda = !closingTag;
                    if (!insideAgenda)
                    {
                        agendaTaskCount = 0;
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (insideAgenda && normalizedTagName == QStringLiteral("task"))
                {
                    if (!closingTag)
                    {
                        if (agendaTaskCount > 0)
                        {
                            logicalText += QLatin1Char('\n');
                        }
                        ++agendaTaskCount;
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (normalizedTagName == QStringLiteral("resource"))
                {
                    if (!closingTag)
                    {
                        logicalText += resourceLogicalPlaceholderText();
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (normalizedTagName == QStringLiteral("tag"))
                {
                    if (!closingTag)
                    {
                        logicalText += QLatin1Char('#');
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (htmlTagProducesLogicalBreak(tagToken))
                {
                    logicalText += QLatin1Char('\n');
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                sourceOffset = tagEnd + 1;
                continue;
            }
        }

        const int entityLength = htmlEntityLengthAt(normalizedText, sourceOffset);
        if (entityLength > 0)
        {
            logicalText += decodedHtmlEntityText(
                QStringView(normalizedText.constData() + sourceOffset, entityLength));
            sourceOffset += entityLength;
            continue;
        }

        logicalText += ch;
        ++sourceOffset;
    }

    return logicalText;
}

int ContentsLogicalTextBridge::countLogicalLines(const QString& text) noexcept
{
    int lineCount = 1;
    for (int index = 0; index < text.size(); ++index)
    {
        if (text.at(index) == QLatin1Char('\n'))
        {
            ++lineCount;
        }
    }

    return lineCount;
}

QVector<int> ContentsLogicalTextBridge::buildLogicalToSourceOffsets(const QString& text, const int logicalTextLength)
{
    const QString normalizedText = normalizeLineEndings(text);
    QVector<int> offsets;
    const int safeLogicalTextLength = std::max(0, logicalTextLength);
    offsets.reserve(safeLogicalTextLength + 1);
    offsets.push_back(0);

    bool insideAgenda = false;
    int agendaTaskCount = 0;
    int logicalOffset = 0;
    int sourceOffset = 0;
    while (sourceOffset < normalizedText.size() && logicalOffset < safeLogicalTextLength)
    {
        const QChar ch = normalizedText.at(sourceOffset);
        if (ch == QLatin1Char('<'))
        {
            const int tagEnd = normalizedText.indexOf(QLatin1Char('>'), sourceOffset + 1);
            if (tagEnd > sourceOffset)
            {
                const QStringView tagToken(normalizedText.constData() + sourceOffset, tagEnd - sourceOffset + 1);
                const QString normalizedTagName = normalizedHtmlTagName(tagToken);
                const bool closingTag = isClosingHtmlTagToken(tagToken);

                if (normalizedTagName == QStringLiteral("agenda"))
                {
                    insideAgenda = !closingTag;
                    if (!insideAgenda)
                    {
                        agendaTaskCount = 0;
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (insideAgenda && normalizedTagName == QStringLiteral("task"))
                {
                    if (!closingTag)
                    {
                        if (agendaTaskCount > 0 && logicalOffset < safeLogicalTextLength)
                        {
                            ++logicalOffset;
                            offsets.push_back(
                                advanceSourceOffsetPastClosingInlineBoundaryTags(
                                    normalizedText,
                                    tagEnd + 1));
                        }
                        ++agendaTaskCount;
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (normalizedTagName == QStringLiteral("resource"))
                {
                    if (!closingTag)
                    {
                        for (int placeholderIndex = 0;
                             placeholderIndex < kResourceLogicalPlaceholderLength && logicalOffset < safeLogicalTextLength;
                             ++placeholderIndex)
                        {
                            ++logicalOffset;
                            offsets.push_back(
                                advanceSourceOffsetPastClosingInlineBoundaryTags(
                                    normalizedText,
                                    tagEnd + 1));
                        }
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (normalizedTagName == QStringLiteral("tag"))
                {
                    if (!closingTag && logicalOffset < safeLogicalTextLength)
                    {
                        ++logicalOffset;
                        offsets.push_back(
                            advanceSourceOffsetPastClosingInlineBoundaryTags(
                                normalizedText,
                                tagEnd + 1));
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }

                if (htmlTagProducesLogicalBreak(tagToken))
                {
                    ++logicalOffset;
                    offsets.push_back(
                        advanceSourceOffsetPastClosingInlineBoundaryTags(
                            normalizedText,
                            tagEnd + 1));
                }
                sourceOffset = tagEnd + 1;
                continue;
            }
        }

        const int entityLength = htmlEntityLengthAt(normalizedText, sourceOffset);
        if (entityLength > 0)
        {
            sourceOffset += entityLength;
            ++logicalOffset;
            offsets.push_back(
                advanceSourceOffsetPastClosingInlineBoundaryTags(
                    normalizedText,
                    sourceOffset));
            continue;
        }

        ++sourceOffset;
        ++logicalOffset;
        offsets.push_back(
            advanceSourceOffsetPastClosingInlineBoundaryTags(
                normalizedText,
                sourceOffset));
    }

    while (offsets.size() < safeLogicalTextLength + 1)
    {
        offsets.push_back(normalizedText.size());
    }

    if (offsets.size() > safeLogicalTextLength + 1)
    {
        offsets.resize(safeLogicalTextLength + 1);
    }

    return offsets;
}

int ContentsLogicalTextBridge::logicalOffsetForSourceOffsetInText(const QString& text, const int sourceOffset)
{
    const QString normalizedText = normalizeLineEndings(text);
    const int boundedSourceOffset =
        std::clamp(sourceOffset, 0, boundedQStringSize(normalizedText));

    bool insideAgenda = false;
    int agendaTaskCount = 0;
    int logicalOffset = 0;
    int cursor = 0;
    while (cursor < normalizedText.size() && cursor < boundedSourceOffset)
    {
        const QChar ch = normalizedText.at(cursor);
        if (ch == QLatin1Char('<'))
        {
            const int tagEnd = normalizedText.indexOf(QLatin1Char('>'), cursor + 1);
            if (tagEnd > cursor)
            {
                if (boundedSourceOffset < tagEnd + 1)
                {
                    return logicalOffset;
                }

                const QStringView tagToken(normalizedText.constData() + cursor, tagEnd - cursor + 1);
                const QString normalizedTagName = normalizedHtmlTagName(tagToken);
                const bool closingTag = isClosingHtmlTagToken(tagToken);

                if (normalizedTagName == QStringLiteral("agenda"))
                {
                    insideAgenda = !closingTag;
                    if (!insideAgenda)
                    {
                        agendaTaskCount = 0;
                    }
                    cursor = tagEnd + 1;
                    continue;
                }

                if (insideAgenda && normalizedTagName == QStringLiteral("task"))
                {
                    if (!closingTag)
                    {
                        if (agendaTaskCount > 0)
                        {
                            ++logicalOffset;
                        }
                        ++agendaTaskCount;
                    }
                    cursor = tagEnd + 1;
                    continue;
                }

                if (normalizedTagName == QStringLiteral("resource"))
                {
                    if (!closingTag)
                    {
                        logicalOffset += kResourceLogicalPlaceholderLength;
                    }
                    cursor = tagEnd + 1;
                    continue;
                }

                if (normalizedTagName == QStringLiteral("tag"))
                {
                    if (!closingTag)
                    {
                        ++logicalOffset;
                    }
                    cursor = tagEnd + 1;
                    continue;
                }

                if (htmlTagProducesLogicalBreak(tagToken))
                {
                    ++logicalOffset;
                    cursor = tagEnd + 1;
                    continue;
                }

                cursor = tagEnd + 1;
                continue;
            }
        }

        const int entityLength = htmlEntityLengthAt(normalizedText, cursor);
        if (entityLength > 0)
        {
            if (boundedSourceOffset < cursor + entityLength)
            {
                return logicalOffset;
            }
            cursor += entityLength;
            ++logicalOffset;
            continue;
        }

        ++cursor;
        ++logicalOffset;
    }

    return logicalOffset;
}

void ContentsLogicalTextBridge::refreshTextState()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("logicalTextBridge"),
        QStringLiteral("refreshTextState"),
        QStringLiteral("source=%1").arg(WhatSon::Debug::summarizeText(m_text)));
    const QString nextLogicalText = normalizeLogicalText(m_text);
    if (m_logicalText != nextLogicalText)
    {
        m_logicalText = nextLogicalText;
        emit logicalTextChanged();
    }
    const QVector<int> nextLogicalToSourceOffsets =
        buildLogicalToSourceOffsets(m_text, m_logicalText.size());
    if (m_logicalToSourceOffsets != nextLogicalToSourceOffsets)
    {
        m_logicalToSourceOffsets = nextLogicalToSourceOffsets;
        emit logicalToSourceOffsetsChanged();
    }

    const int nextLineCount = countLogicalLines(m_logicalText);
    if (m_logicalLineCount != nextLineCount)
    {
        m_logicalLineCount = nextLineCount;
        emit logicalLineCountChanged();
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("logicalTextBridge"),
        QStringLiteral("refreshTextState.done"),
        QStringLiteral("lineCount=%1 logicalOffsets=%2 logical=%3")
            .arg(m_logicalLineCount)
            .arg(m_logicalToSourceOffsets.size())
            .arg(WhatSon::Debug::summarizeText(m_logicalText)));
}
