#include "app/models/editor/gutter/ContentsGutterMarkerGeometry.hpp"

#include <QSet>
#include <QStringList>
#include <QVariantMap>

#include <algorithm>

namespace
{
    QStringList splitLinesKeepingEmptyParts(const QString& text)
    {
        return text.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    }

    int intFromEntry(const QVariant& entry, const QString& key, const int fallback)
    {
        bool ok = false;
        const int value = entry.toMap().value(key).toInt(&ok);
        return ok ? value : fallback;
    }

    qreal realFromEntry(const QVariant& entry, const QString& key, const qreal fallback)
    {
        bool ok = false;
        const qreal value = entry.toMap().value(key).toReal(&ok);
        return ok ? value : fallback;
    }
}

ContentsGutterMarkerGeometry::ContentsGutterMarkerGeometry(QObject* parent)
    : QObject(parent)
{
    rebuildMarkerEntries();
}

ContentsGutterMarkerGeometry::~ContentsGutterMarkerGeometry() = default;

bool ContentsGutterMarkerGeometry::editorMounted() const noexcept
{
    return m_editorMounted;
}

void ContentsGutterMarkerGeometry::setEditorMounted(const bool value)
{
    if (m_editorMounted == value)
    {
        return;
    }

    m_editorMounted = value;
    emit editorMountedChanged();
    rebuildMarkerEntries();
}

QString ContentsGutterMarkerGeometry::sourceText() const
{
    return m_sourceText;
}

void ContentsGutterMarkerGeometry::setSourceText(const QString& value)
{
    if (m_sourceText == value)
    {
        return;
    }

    m_sourceText = value;
    emit sourceTextChanged();
    rebuildMarkerEntries();
}

QString ContentsGutterMarkerGeometry::savedSourceText() const
{
    return m_savedSourceText;
}

void ContentsGutterMarkerGeometry::setSavedSourceText(const QString& value)
{
    if (m_savedSourceText == value)
    {
        return;
    }

    m_savedSourceText = value;
    emit savedSourceTextChanged();
    rebuildMarkerEntries();
}

QVariantList ContentsGutterMarkerGeometry::lineNumberEntries() const
{
    return m_lineNumberEntries;
}

void ContentsGutterMarkerGeometry::setLineNumberEntries(const QVariantList& value)
{
    if (m_lineNumberEntries == value)
    {
        return;
    }

    m_lineNumberEntries = value;
    emit lineNumberEntriesChanged();
    rebuildMarkerEntries();
}

int ContentsGutterMarkerGeometry::cursorPosition() const noexcept
{
    return m_cursorPosition;
}

void ContentsGutterMarkerGeometry::setCursorPosition(const int value)
{
    const int normalizedValue = std::max(0, value);
    if (m_cursorPosition == normalizedValue)
    {
        return;
    }

    m_cursorPosition = normalizedValue;
    emit cursorPositionChanged();
    rebuildMarkerEntries();
}

int ContentsGutterMarkerGeometry::lineNumberBaseOffset() const noexcept
{
    return m_lineNumberBaseOffset;
}

void ContentsGutterMarkerGeometry::setLineNumberBaseOffset(const int value)
{
    if (m_lineNumberBaseOffset == value)
    {
        return;
    }

    m_lineNumberBaseOffset = value;
    emit lineNumberBaseOffsetChanged();
    rebuildMarkerEntries();
}

qreal ContentsGutterMarkerGeometry::markerHeight() const noexcept
{
    return m_markerHeight;
}

void ContentsGutterMarkerGeometry::setMarkerHeight(const qreal value)
{
    const qreal normalizedValue = std::max<qreal>(1.0, value);
    if (qFuzzyCompare(m_markerHeight, normalizedValue))
    {
        return;
    }

    m_markerHeight = normalizedValue;
    emit markerHeightChanged();
    rebuildMarkerEntries();
}

int ContentsGutterMarkerGeometry::cursorLineNumber() const noexcept
{
    return m_cursorLineNumber;
}

QVariantList ContentsGutterMarkerGeometry::markerEntries() const
{
    return m_markerEntries;
}

void ContentsGutterMarkerGeometry::refresh()
{
    rebuildMarkerEntries();
}

int ContentsGutterMarkerGeometry::sourceLineIndexForOffset(const int offset) const noexcept
{
    const int clampedOffset = std::clamp(offset, 0, static_cast<int>(m_sourceText.size()));
    int lineIndex = 0;
    for (int index = 0; index < clampedOffset; ++index)
    {
        if (m_sourceText.at(index) == QLatin1Char('\n'))
        {
            ++lineIndex;
        }
    }
    return lineIndex;
}

QList<int> ContentsGutterMarkerGeometry::unsavedLineNumbers() const
{
    if (m_sourceText == m_savedSourceText)
    {
        return {};
    }

    const QStringList currentLines = splitLinesKeepingEmptyParts(m_sourceText);
    const QStringList savedLines = splitLinesKeepingEmptyParts(m_savedSourceText);
    const int currentLineCount = std::max(1, static_cast<int>(currentLines.size()));
    const int savedLineCount = std::max(1, static_cast<int>(savedLines.size()));
    const int maxLineCount = std::max(currentLineCount, savedLineCount);

    QSet<int> changedLines;
    for (int lineIndex = 0; lineIndex < maxLineCount; ++lineIndex)
    {
        const bool hasCurrentLine = lineIndex < currentLines.size();
        const bool hasSavedLine = lineIndex < savedLines.size();
        const QString currentLine = hasCurrentLine ? currentLines.at(lineIndex) : QString();
        const QString savedLine = hasSavedLine ? savedLines.at(lineIndex) : QString();
        if (hasCurrentLine != hasSavedLine || currentLine != savedLine)
        {
            const int visibleLineIndex = hasCurrentLine ? lineIndex : std::max(0, currentLineCount - 1);
            changedLines.insert(visibleLineIndex + m_lineNumberBaseOffset);
        }
    }

    QList<int> sortedLines = changedLines.values();
    std::sort(sortedLines.begin(), sortedLines.end());
    return sortedLines;
}

qreal ContentsGutterMarkerGeometry::effectiveMarkerHeight() const noexcept
{
    return std::max<qreal>(1.0, m_markerHeight);
}

qreal ContentsGutterMarkerGeometry::yForLineNumber(const int lineNumber) const
{
    bool firstEntrySet = false;
    int firstLineNumber = m_lineNumberBaseOffset;
    qreal firstY = 0.0;

    for (const QVariant& entry : m_lineNumberEntries)
    {
        const int entryLineNumber = intFromEntry(entry, QStringLiteral("lineNumber"), m_lineNumberBaseOffset);
        const qreal entryY = realFromEntry(entry, QStringLiteral("y"), 0.0);
        if (!firstEntrySet)
        {
            firstEntrySet = true;
            firstLineNumber = entryLineNumber;
            firstY = entryY;
        }

        if (entryLineNumber == lineNumber)
        {
            return entryY;
        }
    }

    if (firstEntrySet)
    {
        return firstY + (static_cast<qreal>(lineNumber - firstLineNumber) * effectiveMarkerHeight());
    }

    return static_cast<qreal>(lineNumber - m_lineNumberBaseOffset) * effectiveMarkerHeight();
}

qreal ContentsGutterMarkerGeometry::heightForLineSpan(const int startLineNumber, const int lineSpan) const
{
    if (lineSpan <= 1)
    {
        return effectiveMarkerHeight();
    }

    const qreal startY = yForLineNumber(startLineNumber);
    const qreal endY = yForLineNumber(startLineNumber + lineSpan - 1);
    return std::max(effectiveMarkerHeight(), (endY - startY) + effectiveMarkerHeight());
}

QVariantMap ContentsGutterMarkerGeometry::markerEntry(
    const QString& type,
    const int lineNumber,
    const int lineSpan) const
{
    QVariantMap entry;
    entry.insert(QStringLiteral("type"), type);
    entry.insert(QStringLiteral("lineNumber"), lineNumber);
    entry.insert(QStringLiteral("lineSpan"), std::max(1, lineSpan));
    entry.insert(QStringLiteral("y"), yForLineNumber(lineNumber));
    entry.insert(QStringLiteral("height"), heightForLineSpan(lineNumber, lineSpan));
    return entry;
}

void ContentsGutterMarkerGeometry::rebuildMarkerEntries()
{
    const int nextCursorLineNumber = m_editorMounted
        ? m_lineNumberBaseOffset + sourceLineIndexForOffset(m_cursorPosition)
        : -1;

    QVariantList nextEntries;
    if (m_editorMounted)
    {
        const QList<int> changedLines = unsavedLineNumbers();
        int index = 0;
        while (index < changedLines.size())
        {
            const int startLine = changedLines.at(index);
            int span = 1;
            while (index + span < changedLines.size()
                   && changedLines.at(index + span) == startLine + span)
            {
                ++span;
            }
            nextEntries.append(markerEntry(QStringLiteral("unsaved"), startLine, span));
            index += span;
        }

        nextEntries.append(markerEntry(QStringLiteral("cursor"), nextCursorLineNumber, 1));
    }

    const bool cursorLineChanged = m_cursorLineNumber != nextCursorLineNumber;
    const bool markerEntriesChangedValue = m_markerEntries != nextEntries;
    m_cursorLineNumber = nextCursorLineNumber;
    m_markerEntries = nextEntries;

    if (cursorLineChanged)
    {
        emit cursorLineNumberChanged();
    }
    if (markerEntriesChangedValue)
    {
        emit markerEntriesChanged();
    }
}
