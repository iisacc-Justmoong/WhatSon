#include "app/models/content/display/ContentsDisplayMinimapCoordinator.hpp"

#include <QtGlobal>

ContentsDisplayMinimapCoordinator::ContentsDisplayMinimapCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayMinimapCoordinator::~ContentsDisplayMinimapCoordinator() = default;

bool ContentsDisplayMinimapCoordinator::structuredHostGeometryActive() const noexcept { return m_structuredHostGeometryActive; }
void ContentsDisplayMinimapCoordinator::setStructuredHostGeometryActive(const bool active)
{
    if (m_structuredHostGeometryActive == active)
        return;
    m_structuredHostGeometryActive = active;
    emit structuredHostGeometryActiveChanged();
}

double ContentsDisplayMinimapCoordinator::editorLineHeight() const noexcept { return m_editorLineHeight; }
void ContentsDisplayMinimapCoordinator::setEditorLineHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorLineHeight, normalized))
        return;
    m_editorLineHeight = normalized;
    emit editorLineHeightChanged();
}

double ContentsDisplayMinimapCoordinator::editorDocumentStartY() const noexcept { return m_editorDocumentStartY; }
void ContentsDisplayMinimapCoordinator::setEditorDocumentStartY(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorDocumentStartY, normalized))
        return;
    m_editorDocumentStartY = normalized;
    emit editorDocumentStartYChanged();
}

int ContentsDisplayMinimapCoordinator::logicalLineCount() const noexcept { return m_logicalLineCount; }
void ContentsDisplayMinimapCoordinator::setLogicalLineCount(const int value)
{
    const int normalized = qMax(1, value);
    if (m_logicalLineCount == normalized)
        return;
    m_logicalLineCount = normalized;
    emit logicalLineCountChanged();
}

QVariantMap ContentsDisplayMinimapCoordinator::currentCursorVisualRowRectFromStructuredRect(
    const QVariantMap& rawRect,
    const int fallbackLineNumber,
    const double fallbackLineY,
    const double fallbackLineHeight) const
{
    const int safeLineNumber = qMax(1, qMin(m_logicalLineCount, fallbackLineNumber));
    Q_UNUSED(safeLineNumber)
    return normalizeCursorRect(rawRect, qMax(0.0, fallbackLineY), qMax(1.0, fallbackLineHeight), false);
}

QVariantMap ContentsDisplayMinimapCoordinator::currentCursorVisualRowRectFromTextRect(
    const QVariantMap& rawRect,
    const int safeOffset,
    const double fallbackDocumentY) const
{
    Q_UNUSED(safeOffset)
    return normalizeCursorRect(rawRect, qMax(0.0, fallbackDocumentY), qMax(1.0, m_editorLineHeight), true);
}

QVariantList ContentsDisplayMinimapCoordinator::buildStructuredMinimapLineGroupsForRange(
    const QVariantList& lineEntries,
    const int startLineNumber,
    const int endLineNumber) const
{
    if (lineEntries.isEmpty())
        return {};

    const int safeStartLine = qMax(1, qMin(lineEntries.size(), startLineNumber));
    const int safeEndLine = qMax(safeStartLine, qMin(lineEntries.size(), endLineNumber));
    QVariantList groups;
    for (int lineIndex = safeStartLine - 1; lineIndex < safeEndLine; ++lineIndex)
    {
        const QVariantMap entry = lineEntries.at(lineIndex).toMap();
        groups.push_back(minimapGroup(
            lineIndex + 1,
            qMax(0, entry.value(QStringLiteral("charCount")).toInt()),
            m_editorDocumentStartY + qMax(0.0, entry.value(QStringLiteral("contentY")).toDouble()),
            qMax(m_editorLineHeight, entry.value(QStringLiteral("contentHeight")).toDouble() > 0.0
                                          ? entry.value(QStringLiteral("contentHeight")).toDouble()
                                          : m_editorLineHeight),
            entry.value(QStringLiteral("minimapVisualKind")).toString().isEmpty()
                ? QStringLiteral("text")
                : entry.value(QStringLiteral("minimapVisualKind")).toString(),
            qMax(0, entry.value(QStringLiteral("minimapRowCharCount")).toInt()),
            qMax(1, entry.value(QStringLiteral("rowCount")).toInt())));
    }
    return groups;
}

QVariantList ContentsDisplayMinimapCoordinator::buildFallbackMinimapLineGroupsForRange(
    const QVariantList& lineCharacterCounts,
    const QVariantList& lineDocumentYs,
    const QVariantList& lineVisualHeights,
    const int startLineNumber,
    const int endLineNumber) const
{
    const int safeStartLine = qMax(1, qMin(m_logicalLineCount, startLineNumber));
    const int safeEndLine = qMax(safeStartLine, qMin(m_logicalLineCount, endLineNumber));
    QVariantList groups;
    for (int lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber)
    {
        const int index = lineNumber - 1;
        const int charCount = index < lineCharacterCounts.size() ? qMax(0, lineCharacterCounts.at(index).toInt()) : 0;
        const double contentY = index < lineDocumentYs.size() ? qMax(0.0, lineDocumentYs.at(index).toDouble()) : 0.0;
        const double contentHeight = index < lineVisualHeights.size() ? qMax(1.0, lineVisualHeights.at(index).toDouble()) : qMax(1.0, m_editorLineHeight);
        groups.push_back(minimapGroup(
            lineNumber,
            charCount,
            m_editorDocumentStartY + contentY,
            contentHeight,
            QStringLiteral("text"),
            0,
            qMax(1, qCeil(contentHeight / qMax(1.0, m_editorLineHeight)))));
    }
    if (groups.isEmpty())
        groups.push_back(minimapGroup(1, 0, m_editorDocumentStartY, qMax(1.0, m_editorLineHeight)));
    return groups;
}

QVariantList ContentsDisplayMinimapCoordinator::buildEditorMinimapLineGroupsForRange(
    const QVariantList& lineCharacterCounts,
    const QVariantList& lineStartOffsets,
    const QVariantList& fallbackLineDocumentYs,
    const QVariantList& fallbackLineVisualHeights,
    const QVariantList& editorRects,
    const int logicalTextLength,
    const int startLineNumber,
    const int endLineNumber,
    const double editorWidth,
    const double editorContentHeight) const
{
    if (editorWidth <= 0.0 || editorContentHeight <= 0.0 || editorRects.isEmpty())
        return buildFallbackMinimapLineGroupsForRange(lineCharacterCounts, fallbackLineDocumentYs, fallbackLineVisualHeights, startLineNumber, endLineNumber);

    const int safeStartLine = qMax(1, qMin(m_logicalLineCount, startLineNumber));
    const int safeEndLine = qMax(safeStartLine, qMin(m_logicalLineCount, endLineNumber));
    QVariantList groups;
    for (int lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber)
    {
        const int index = lineNumber - 1;
        const QVariantMap currentRect = index < editorRects.size() ? editorRects.at(index).toMap() : QVariantMap();
        const QVariantMap nextRect = lineNumber < editorRects.size() ? editorRects.at(lineNumber).toMap() : QVariantMap();
        const double fallbackCurrentY = index < fallbackLineDocumentYs.size() ? qMax(0.0, fallbackLineDocumentYs.at(index).toDouble()) : 0.0;
        const double currentRectY = currentRect.value(QStringLiteral("y")).toDouble();
        const double resolvedCurrentY = qIsFinite(currentRectY) ? currentRectY : fallbackCurrentY;

        double contentHeight = qMax(1.0, m_editorLineHeight);
        if (lineNumber < m_logicalLineCount)
        {
            const double nextRectY = nextRect.value(QStringLiteral("y")).toDouble();
            const double resolvedNextY = qIsFinite(nextRectY)
                ? nextRectY
                : resolvedCurrentY + qMax(1.0, m_editorLineHeight);
            contentHeight = qMax(qMax(1.0, m_editorLineHeight), resolvedNextY - resolvedCurrentY);
        }
        else
        {
            const double nextRectY = nextRect.value(QStringLiteral("y")).toDouble();
            const double nextRectHeight = nextRect.value(QStringLiteral("height")).toDouble();
            const double resolvedNextY = qIsFinite(nextRectY) ? nextRectY : resolvedCurrentY;
            const double resolvedNextHeight = qIsFinite(nextRectHeight) ? nextRectHeight : qMax(1.0, m_editorLineHeight);
            contentHeight = qMax(qMax(1.0, m_editorLineHeight), resolvedNextY + qMax(qMax(1.0, m_editorLineHeight), resolvedNextHeight) - resolvedCurrentY);
        }

        const int charCount = index < lineCharacterCounts.size() ? qMax(0, lineCharacterCounts.at(index).toInt()) : 0;
        groups.push_back(minimapGroup(
            lineNumber,
            charCount,
            m_editorDocumentStartY + resolvedCurrentY,
            contentHeight,
            QStringLiteral("text"),
            0,
            qMax(1, qCeil(contentHeight / qMax(1.0, m_editorLineHeight)))));
    }

    if (groups.isEmpty())
        return buildFallbackMinimapLineGroupsForRange(lineCharacterCounts, fallbackLineDocumentYs, fallbackLineVisualHeights, startLineNumber, endLineNumber);

    Q_UNUSED(lineStartOffsets)
    Q_UNUSED(logicalTextLength)
    return groups;
}

QVariantMap ContentsDisplayMinimapCoordinator::buildNextMinimapSnapshotPlan(
    const QVariant& currentLineGroups,
    const QString& currentLineGroupsNoteId,
    const QString& currentNoteId,
    const QString& previousSourceText,
    const QString& currentSourceText,
    const bool forceFullRefresh,
    const bool noteEntryRefreshPending,
    const int structuredLineCount,
    const int plainLogicalLineCount) const
{
    const QVariantList existingGroups = currentLineGroups.toList();
    const int expectedLineCount = m_structuredHostGeometryActive
        ? qMax(1, structuredLineCount)
        : qMax(1, plainLogicalLineCount);

    QVariantMap plan;
    plan.insert(QStringLiteral("reuseExisting"), false);
    plan.insert(QStringLiteral("requiresFullRebuild"), false);
    plan.insert(QStringLiteral("replacementStartLine"), 1);
    plan.insert(QStringLiteral("replacementEndLine"), expectedLineCount);
    plan.insert(QStringLiteral("previousStartLine"), 1);
    plan.insert(QStringLiteral("previousEndLine"), expectedLineCount);

    const bool invalidExistingGroups = existingGroups.isEmpty() || existingGroups.size() != expectedLineCount;
    if (forceFullRefresh
        || noteEntryRefreshPending
        || currentLineGroupsNoteId != currentNoteId
        || invalidExistingGroups
        || previousSourceText.isEmpty())
    {
        plan.insert(QStringLiteral("requiresFullRebuild"), true);
        return plan;
    }

    const QStringList previousLines = previousSourceText.split(u'\n');
    const QStringList currentLines = currentSourceText.split(u'\n');

    int prefix = 0;
    const int previousCount = previousLines.size();
    const int currentCount = currentLines.size();
    while (prefix < previousCount && prefix < currentCount
           && previousLines.at(prefix) == currentLines.at(prefix))
    {
        ++prefix;
    }

    if (prefix == previousCount && prefix == currentCount)
    {
        plan.insert(QStringLiteral("reuseExisting"), true);
        return plan;
    }

    int previousSuffix = previousCount - 1;
    int currentSuffix = currentCount - 1;
    while (previousSuffix >= prefix && currentSuffix >= prefix
           && previousLines.at(previousSuffix) == currentLines.at(currentSuffix))
    {
        --previousSuffix;
        --currentSuffix;
    }

    const int replacementStartLine = qMax(1, prefix + 1);
    const int replacementEndLine = qMax(replacementStartLine, currentSuffix + 1);
    const int previousStartLine = qMax(1, prefix + 1);
    const int previousEndLine = qMax(previousStartLine, previousSuffix + 1);

    plan.insert(QStringLiteral("replacementStartLine"), replacementStartLine);
    plan.insert(QStringLiteral("replacementEndLine"), replacementEndLine);
    plan.insert(QStringLiteral("previousStartLine"), previousStartLine);
    plan.insert(QStringLiteral("previousEndLine"), previousEndLine);
    return plan;
}

QVariantMap ContentsDisplayMinimapCoordinator::normalizeCursorRect(
    const QVariantMap& rawRect,
    const double fallbackY,
    const double fallbackHeight,
    const bool preserveWidth) const
{
    QVariantMap rect;
    rect.insert(QStringLiteral("height"), qMax(1.0, rawRect.value(QStringLiteral("height")).toDouble() > 0.0
                                                       ? rawRect.value(QStringLiteral("height")).toDouble()
                                                       : fallbackHeight));
    rect.insert(QStringLiteral("width"), preserveWidth ? qMax(0.0, rawRect.value(QStringLiteral("width")).toDouble()) : 0.0);
    rect.insert(QStringLiteral("y"), qMax(0.0, qIsFinite(rawRect.value(QStringLiteral("y")).toDouble())
                                                    ? rawRect.value(QStringLiteral("y")).toDouble()
                                                    : fallbackY));
    return rect;
}

QVariantMap ContentsDisplayMinimapCoordinator::minimapGroup(
    const int lineNumber,
    const int charCount,
    const double contentY,
    const double contentHeight,
    const QString& minimapVisualKind,
    const int minimapRowCharCount,
    const int rowCount) const
{
    QVariantMap group;
    group.insert(QStringLiteral("charCount"), qMax(0, charCount));
    group.insert(QStringLiteral("contentHeight"), qMax(1.0, contentHeight));
    group.insert(QStringLiteral("contentY"), qMax(0.0, contentY));
    group.insert(QStringLiteral("lineNumber"), qMax(1, lineNumber));
    group.insert(QStringLiteral("minimapRowCharCount"), qMax(0, minimapRowCharCount));
    group.insert(QStringLiteral("minimapVisualKind"), minimapVisualKind.isEmpty() ? QStringLiteral("text") : minimapVisualKind);
    group.insert(QStringLiteral("rowCount"), qMax(1, rowCount));
    return group;
}
