#include "app/models/editor/display/ContentsDisplayMinimapCoordinator.hpp"

#include <QtGlobal>

#include <QVector>

namespace
{
constexpr double kStructuredMinimapSyntheticAvailableWidth = 160.0;
constexpr double kStructuredMinimapResourceWidth = 148.0;
constexpr double kStructuredMinimapBreakWidth = 52.0;
constexpr double kStructuredMinimapMinimumTextWidth = 12.0;

QString normalizedMinimapPlainText(QString text)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text.replace(QChar::LineSeparator, QLatin1Char('\n'));
    text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    return text;
}

QString normalizedMinimapVisualKind(const QVariantMap& entry)
{
    const QString visualKind = entry.value(QStringLiteral("minimapVisualKind")).toString().trimmed().toLower();
    if (!visualKind.isEmpty())
        return visualKind;

    const QString typeName = entry.value(QStringLiteral("type")).toString().trimmed().toLower();
    if (typeName == QStringLiteral("resource"))
        return QStringLiteral("block");
    return QStringLiteral("text");
}

QString normalizedMinimapBlockType(const QVariantMap& entry)
{
    return entry.value(QStringLiteral("type")).toString().trimmed().toLower();
}

QString normalizedMinimapBlockPlainText(const QVariantMap& entry)
{
    if (entry.contains(QStringLiteral("plainText")))
        return normalizedMinimapPlainText(entry.value(QStringLiteral("plainText")).toString());
    if (entry.contains(QStringLiteral("sourceText")))
        return normalizedMinimapPlainText(entry.value(QStringLiteral("sourceText")).toString());
    return {};
}

int parsedMinimapCharCount(const QVariantMap& entry)
{
    QString visibleText = normalizedMinimapBlockPlainText(entry);
    visibleText.remove(QLatin1Char('\n'));
    visibleText.remove(QLatin1Char('\r'));
    const int visibleCharCount = qMax(0, visibleText.size());
    if (visibleCharCount > 0)
        return visibleCharCount;
    return qMax(0, entry.value(QStringLiteral("minimapRepresentativeCharCount")).toInt());
}

double parsedMinimapWeight(const QVariantMap& entry)
{
    const QString visualKind = normalizedMinimapVisualKind(entry);
    const QString typeName = normalizedMinimapBlockType(entry);
    if (visualKind == QStringLiteral("block"))
        return 2.0;
    if (typeName == QStringLiteral("break"))
        return 0.75;
    return 1.0;
}

double parsedMinimapContentWidth(const QVariantMap& entry, const int charCount)
{
    const QString visualKind = normalizedMinimapVisualKind(entry);
    const QString typeName = normalizedMinimapBlockType(entry);
    if (visualKind == QStringLiteral("block"))
        return kStructuredMinimapResourceWidth;
    if (typeName == QStringLiteral("break"))
        return kStructuredMinimapBreakWidth;
    if (charCount <= 0)
        return 0.0;
    return qMin(
        kStructuredMinimapSyntheticAvailableWidth,
        qMax(kStructuredMinimapMinimumTextWidth, static_cast<double>(charCount)));
}

QVariantMap structuredMinimapSnapshotEntry(const QVariantMap& blockEntry, const int lineNumber)
{
    const QString sourceText = normalizedMinimapPlainText(blockEntry.value(QStringLiteral("sourceText")).toString());
    const QString plainText = normalizedMinimapBlockPlainText(blockEntry);
    const int sourceStart = qMax(0, blockEntry.value(QStringLiteral("sourceStart")).toInt());
    const int sourceEnd = qMax(sourceStart, blockEntry.value(QStringLiteral("sourceEnd")).toInt());
    const QString visualKind = normalizedMinimapVisualKind(blockEntry);
    const int charCount = parsedMinimapCharCount(blockEntry);
    QVariantMap entry;
    entry.insert(QStringLiteral("charCount"), charCount);
    entry.insert(QStringLiteral("lineNumber"), qMax(1, lineNumber));
    entry.insert(
        QStringLiteral("minimapRepresentativeCharCount"),
        qMax(0, blockEntry.value(QStringLiteral("minimapRepresentativeCharCount")).toInt()));
    entry.insert(QStringLiteral("minimapVisualKind"), visualKind);
    entry.insert(QStringLiteral("plainText"), plainText);
    entry.insert(
        QStringLiteral("snapshotToken"),
        QStringLiteral("block|%1|%2|%3|%4|%5")
            .arg(visualKind)
            .arg(sourceStart)
            .arg(sourceEnd)
            .arg(charCount)
            .arg(sourceText));
    entry.insert(QStringLiteral("type"), normalizedMinimapBlockType(blockEntry));
    return entry;
}
}

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

QVariantList ContentsDisplayMinimapCoordinator::buildStructuredMinimapSnapshotEntries(
    const QVariantList& blockEntries) const
{
    QVariantList entries;
    entries.reserve(blockEntries.size());
    for (int blockIndex = 0; blockIndex < blockEntries.size(); ++blockIndex)
    {
        entries.push_back(
            structuredMinimapSnapshotEntry(
                blockEntries.at(blockIndex).toMap(),
                blockIndex + 1));
    }

    if (entries.isEmpty())
    {
        entries.push_back(structuredMinimapSnapshotEntry(QVariantMap{}, 1));
    }

    return entries;
}

QVariantList ContentsDisplayMinimapCoordinator::buildStructuredMinimapLineGroupsForRange(
    const QVariantList& snapshotEntries,
    const int startLineNumber,
    const int endLineNumber,
    const double documentContentHeight) const
{
    if (snapshotEntries.isEmpty())
        return {};

    const int safeStartLine = qMax(1, qMin(snapshotEntries.size(), startLineNumber));
    const int safeEndLine = qMax(safeStartLine, qMin(snapshotEntries.size(), endLineNumber));
    const double safeFallbackContentHeight = qMax(
        qMax(1.0, m_editorLineHeight),
        static_cast<double>(snapshotEntries.size()) * qMax(1.0, m_editorLineHeight));
    const double safeDocumentContentHeight = qMax(safeFallbackContentHeight, documentContentHeight);

    QVector<double> weights;
    weights.reserve(snapshotEntries.size());
    double totalWeight = 0.0;
    for (const QVariant& rawEntry : snapshotEntries)
    {
        const double weight = qMax(0.1, parsedMinimapWeight(rawEntry.toMap()));
        weights.push_back(weight);
        totalWeight += weight;
    }
    if (totalWeight <= 0.0)
        totalWeight = static_cast<double>(qMax(1, snapshotEntries.size()));

    QVariantList groups;
    double prefixWeight = 0.0;
    for (int entryIndex = 0; entryIndex < snapshotEntries.size(); ++entryIndex)
    {
        const double entryWeight = entryIndex < weights.size() ? weights.at(entryIndex) : 1.0;
        if (entryIndex + 1 < safeStartLine)
        {
            prefixWeight += entryWeight;
            continue;
        }
        if (entryIndex + 1 > safeEndLine)
            break;

        const QVariantMap entry = snapshotEntries.at(entryIndex).toMap();
        const double startRatio = prefixWeight / totalWeight;
        const double endRatio = (prefixWeight + entryWeight) / totalWeight;
        const double contentY = safeDocumentContentHeight * startRatio;
        const double contentHeight = qMax(
            1.0,
            (safeDocumentContentHeight * endRatio) - contentY);
        const int charCount = qMax(0, entry.value(QStringLiteral("charCount")).toInt());
        const int minimapRepresentativeCharCount = qMax(
            0,
            entry.value(QStringLiteral("minimapRepresentativeCharCount")).toInt());
        const QString minimapVisualKind = normalizedMinimapVisualKind(entry);
        const double contentWidth = parsedMinimapContentWidth(entry, charCount);

        QVariantList visualRowWidths;
        visualRowWidths.push_back(contentWidth);
        groups.push_back(minimapGroup(
            entryIndex + 1,
            charCount,
            m_editorDocumentStartY + contentY,
            contentHeight,
            contentWidth,
            kStructuredMinimapSyntheticAvailableWidth,
            visualRowWidths,
            minimapVisualKind,
            minimapRepresentativeCharCount > 0 ? minimapRepresentativeCharCount : charCount,
            1));
        prefixWeight += entryWeight;
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
            0.0,
            0.0,
            QVariantList(),
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
            0.0,
            0.0,
            QVariantList(),
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
    const double contentWidth,
    const double contentAvailableWidth,
    const QVariantList& visualRowWidths,
    const QString& minimapVisualKind,
    const int minimapRowCharCount,
    const int rowCount) const
{
    const double normalizedContentWidth = qMax(0.0, contentWidth);
    const double normalizedContentAvailableWidth = qMax(normalizedContentWidth, qMax(0.0, contentAvailableWidth));
    QVariantList normalizedVisualRowWidths;
    normalizedVisualRowWidths.reserve(visualRowWidths.size());
    for (const QVariant& rawWidth : visualRowWidths)
        normalizedVisualRowWidths.push_back(qMax(0.0, rawWidth.toDouble()));
    if (normalizedVisualRowWidths.isEmpty())
        normalizedVisualRowWidths.push_back(normalizedContentWidth);

    QVariantMap group;
    group.insert(QStringLiteral("charCount"), qMax(0, charCount));
    group.insert(QStringLiteral("contentAvailableWidth"), normalizedContentAvailableWidth);
    group.insert(QStringLiteral("contentHeight"), qMax(1.0, contentHeight));
    group.insert(QStringLiteral("contentWidth"), normalizedContentWidth);
    group.insert(QStringLiteral("contentY"), qMax(0.0, contentY));
    group.insert(QStringLiteral("lineNumber"), qMax(1, lineNumber));
    group.insert(QStringLiteral("minimapRowCharCount"), qMax(0, minimapRowCharCount));
    group.insert(QStringLiteral("minimapVisualKind"), minimapVisualKind.isEmpty() ? QStringLiteral("text") : minimapVisualKind);
    group.insert(QStringLiteral("rowCount"), qMax(1, rowCount));
    group.insert(QStringLiteral("visualRowWidths"), normalizedVisualRowWidths);
    return group;
}
