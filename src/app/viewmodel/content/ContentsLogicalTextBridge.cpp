#include "ContentsLogicalTextBridge.hpp"

#include <algorithm>

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
                               : m_text.size();
    return std::max(0, nextOffset - startOffset - (safeIndex + 1 < m_logicalLineStartOffsets.size() ? 1 : 0));
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

void ContentsLogicalTextBridge::refreshTextState()
{
    const QVariantList nextOffsets = buildLogicalLineOffsets(m_text);
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
