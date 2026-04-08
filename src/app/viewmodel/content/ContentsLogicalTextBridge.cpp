#include "ContentsLogicalTextBridge.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"

#include <QChar>
#include <QStringList>
#include <QStringView>
#include <algorithm>
#include <limits>

namespace
{
    int boundedQStringSize(const QString& text) noexcept
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        return static_cast<int>(std::min<qsizetype>(text.size(), maxIntSize));
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

    bool htmlTagProducesLogicalBreak(const QStringView tagToken)
    {
        return normalizedHtmlTagName(tagToken) == QStringLiteral("br");
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

} // namespace

ContentsLogicalTextBridge::ContentsLogicalTextBridge(QObject* parent)
    : QObject(parent)
{
    refreshTextState();
}

ContentsLogicalTextBridge::~ContentsLogicalTextBridge() = default;

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
    if (m_text == text)
    {
        return;
    }

    m_text = text;
    emit textChanged();
    refreshTextState();
}

QVariantList ContentsLogicalTextBridge::logicalLineStartOffsets() const
{
    return m_logicalLineStartOffsets;
}

int ContentsLogicalTextBridge::logicalLineCount() const noexcept
{
    return m_logicalLineCount;
}

int ContentsLogicalTextBridge::logicalLineNumberForOffset(int offset) const noexcept
{
    if (m_logicalLineStartOffsets.isEmpty())
    {
        return 1;
    }

    const int safeOffset = std::max(0, offset);
    int low = 0;
    int high = m_logicalLineStartOffsets.size() - 1;
    int best = 0;
    while (low <= high)
    {
        const int middle = (low + high) / 2;
        const int middleOffset = std::max(0, m_logicalLineStartOffsets.at(middle).toInt());
        if (middleOffset <= safeOffset)
        {
            best = middle;
            low = middle + 1;
        }
        else
        {
            high = middle - 1;
        }
    }
    return best + 1;
}

int ContentsLogicalTextBridge::logicalLineStartOffsetAt(int index) const noexcept
{
    if (m_logicalLineStartOffsets.isEmpty())
    {
        return 0;
    }

    const int maxIndex = static_cast<int>(m_logicalLineStartOffsets.size()) - 1;
    const int safeIndex = std::clamp(index, 0, maxIndex);
    return std::max(0, m_logicalLineStartOffsets.at(safeIndex).toInt());
}

int ContentsLogicalTextBridge::logicalLineCharacterCountAt(int index) const noexcept
{
    if (m_logicalLineStartOffsets.isEmpty())
    {
        return 0;
    }

    const int maxIndex = static_cast<int>(m_logicalLineStartOffsets.size()) - 1;
    const int safeIndex = std::clamp(index, 0, maxIndex);
    const int startOffset = logicalLineStartOffsetAt(safeIndex);
    const int nextOffset = safeIndex + 1 < m_logicalLineStartOffsets.size()
                               ? logicalLineStartOffsetAt(safeIndex + 1)
                               : m_logicalText.size();
    return std::max(0, nextOffset - startOffset - (safeIndex + 1 < m_logicalLineStartOffsets.size() ? 1 : 0));
}

int ContentsLogicalTextBridge::logicalLengthForSourceText(const QString& text) const
{
    return boundedQStringSize(normalizeLogicalText(text));
}

QVariantList ContentsLogicalTextBridge::logicalToSourceOffsets() const
{
    QVariantList offsets;
    offsets.reserve(std::max(0, m_logicalToSourceOffsets.size()));
    for (const int offset : m_logicalToSourceOffsets)
    {
        offsets.push_back(offset);
    }
    return offsets;
}

void ContentsLogicalTextBridge::adoptIncrementalState(
    const QString& sourceText,
    const QString& logicalText,
    const QVariantList& logicalLineStartOffsets,
    const QVariantList& logicalToSourceOffsets)
{
    const QString normalizedSourceText = sourceText;
    const QString normalizedLogicalText = logicalText;
    const QVariantList nextLogicalLineStartOffsets =
        logicalLineStartOffsets.isEmpty() ? buildLogicalLineOffsets(normalizedLogicalText) : logicalLineStartOffsets;
    QVector<int> nextLogicalToSourceOffsets = buildIntVector(logicalToSourceOffsets);
    if (nextLogicalToSourceOffsets.isEmpty())
    {
        nextLogicalToSourceOffsets =
            buildLogicalToSourceOffsets(normalizedSourceText, boundedQStringSize(normalizedLogicalText));
    }

    const int nextLogicalLineCount = std::max(1, static_cast<int>(nextLogicalLineStartOffsets.size()));
    const bool textChangedFlag = m_text != normalizedSourceText;
    const bool logicalTextChangedFlag = m_logicalText != normalizedLogicalText;
    const bool logicalOffsetsChangedFlag = m_logicalLineStartOffsets != nextLogicalLineStartOffsets;
    const bool logicalLineCountChangedFlag = m_logicalLineCount != nextLogicalLineCount;

    if (!textChangedFlag
        && !logicalTextChangedFlag
        && !logicalOffsetsChangedFlag
        && !logicalLineCountChangedFlag
        && m_logicalToSourceOffsets == nextLogicalToSourceOffsets)
    {
        return;
    }

    m_text = normalizedSourceText;
    m_logicalText = normalizedLogicalText;
    m_logicalLineStartOffsets = nextLogicalLineStartOffsets;
    m_logicalToSourceOffsets = std::move(nextLogicalToSourceOffsets);
    m_logicalLineCount = nextLogicalLineCount;

    if (textChangedFlag)
    {
        emit textChanged();
    }
    if (logicalTextChangedFlag)
    {
        emit logicalTextChanged();
    }
    if (logicalOffsetsChangedFlag)
    {
        emit logicalLineStartOffsetsChanged();
    }
    if (logicalLineCountChangedFlag)
    {
        emit logicalLineCountChanged();
    }
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

QString ContentsLogicalTextBridge::normalizeLogicalText(const QString& text)
{
    const QString serializedBodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), text);
    return WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(serializedBodyDocument);
}

QVariantList ContentsLogicalTextBridge::buildLogicalLineOffsets(const QString& text)
{
    QVariantList offsets;
    offsets.push_back(0);

    for (int index = 0; index < text.size(); ++index)
    {
        if (text.at(index) == QLatin1Char('\n'))
        {
            offsets.push_back(index + 1);
        }
    }

    return offsets;
}

QVector<int> ContentsLogicalTextBridge::buildIntVector(const QVariantList& values)
{
    QVector<int> converted;
    converted.reserve(std::max(0, values.size()));
    for (const QVariant& value : values)
    {
        converted.push_back(std::max(0, value.toInt()));
    }
    return converted;
}

QVector<int> ContentsLogicalTextBridge::buildLogicalToSourceOffsets(const QString& text, const int logicalTextLength)
{
    QVector<int> offsets;
    const int safeLogicalTextLength = std::max(0, logicalTextLength);
    offsets.reserve(safeLogicalTextLength + 1);
    offsets.push_back(0);

    int logicalOffset = 0;
    int sourceOffset = 0;
    while (sourceOffset < text.size() && logicalOffset < safeLogicalTextLength)
    {
        const QChar ch = text.at(sourceOffset);
        if (ch == QLatin1Char('<'))
        {
            const int tagEnd = text.indexOf(QLatin1Char('>'), sourceOffset + 1);
            if (tagEnd > sourceOffset)
            {
                const QStringView tagToken(text.constData() + sourceOffset, tagEnd - sourceOffset + 1);
                if (htmlTagProducesLogicalBreak(tagToken))
                {
                    ++logicalOffset;
                    offsets.push_back(tagEnd + 1);
                }
                sourceOffset = tagEnd + 1;
                continue;
            }
        }

        const int entityLength = htmlEntityLengthAt(text, sourceOffset);
        if (entityLength > 0)
        {
            sourceOffset += entityLength;
            ++logicalOffset;
            offsets.push_back(sourceOffset);
            continue;
        }

        ++sourceOffset;
        ++logicalOffset;
        offsets.push_back(sourceOffset);
    }

    while (offsets.size() < safeLogicalTextLength + 1)
    {
        offsets.push_back(text.size());
    }

    if (offsets.size() > safeLogicalTextLength + 1)
    {
        offsets.resize(safeLogicalTextLength + 1);
    }

    return offsets;
}

void ContentsLogicalTextBridge::refreshTextState()
{
    const QString nextLogicalText = normalizeLogicalText(m_text);
    if (m_logicalText != nextLogicalText)
    {
        m_logicalText = nextLogicalText;
        emit logicalTextChanged();
    }
    m_logicalToSourceOffsets = buildLogicalToSourceOffsets(m_text, m_logicalText.size());
    const QVariantList nextOffsets = buildLogicalLineOffsets(m_logicalText);
    const int nextLineCount = std::max(1, static_cast<int>(nextOffsets.size()));

    if (m_logicalLineStartOffsets != nextOffsets)
    {
        m_logicalLineStartOffsets = nextOffsets;
        emit logicalLineStartOffsetsChanged();
    }
    if (m_logicalLineCount != nextLineCount)
    {
        m_logicalLineCount = nextLineCount;
        emit logicalLineCountChanged();
    }
}
