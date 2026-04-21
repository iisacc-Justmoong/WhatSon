#include "ContentsStructuredDocumentBlocksModel.hpp"

#include "models/file/WhatSonDebugTrace.hpp"

#include <QMetaType>

#include <algorithm>

namespace
{
    QVariantMap normalizedBlockMap(const QVariant& value)
    {
        return value.typeId() == QMetaType::QVariantMap ? value.toMap() : QVariantMap{};
    }
}

ContentsStructuredDocumentBlocksModel::ContentsStructuredDocumentBlocksModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentBlocksModel"),
        QStringLiteral("ctor"));
}

ContentsStructuredDocumentBlocksModel::~ContentsStructuredDocumentBlocksModel()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentBlocksModel"),
        QStringLiteral("dtor"),
        QStringLiteral("count=%1").arg(m_entries.size()));
}

QVariantList ContentsStructuredDocumentBlocksModel::blocks() const
{
    QVariantList exportedBlocks;
    exportedBlocks.reserve(m_entries.size());
    for (const Entry& entry : m_entries)
    {
        exportedBlocks.push_back(entry.blockData);
    }
    return exportedBlocks;
}

void ContentsStructuredDocumentBlocksModel::setBlocks(const QVariantList& blocks)
{
    QVector<Entry> nextEntries;
    nextEntries.reserve(blocks.size());
    for (const QVariant& blockValue : blocks)
    {
        nextEntries.push_back(entryFromVariant(blockValue));
    }

    if (m_entries == nextEntries)
    {
        return;
    }

    const int previousCount = m_entries.size();
    const int nextCount = static_cast<int>(nextEntries.size());
    const int prefixCount = commonPrefixCount(nextEntries);
    const int suffixCount = commonSuffixCount(nextEntries, prefixCount);
    const int previousMiddleCount = std::max(0, previousCount - prefixCount - suffixCount);
    const int nextMiddleCount = std::max(0, nextCount - prefixCount - suffixCount);
    const int sharedMiddleCount = std::min(previousMiddleCount, nextMiddleCount);
    QVector<int> changedRows;

    for (int row = 0; row < prefixCount; ++row)
    {
        if (replaceEntryIfChanged(row, nextEntries.at(row)))
        {
            changedRows.push_back(row);
        }
    }

    for (int offset = 0; offset < sharedMiddleCount; ++offset)
    {
        const int row = prefixCount + offset;
        if (replaceEntryIfChanged(row, nextEntries.at(row)))
        {
            changedRows.push_back(row);
        }
    }

    if (previousMiddleCount > nextMiddleCount)
    {
        const int removeStartRow = prefixCount + nextMiddleCount;
        const int removeEndRow = prefixCount + previousMiddleCount - 1;
        beginRemoveRows(QModelIndex(), removeStartRow, removeEndRow);
        m_entries.erase(
            m_entries.begin() + removeStartRow,
            m_entries.begin() + removeEndRow + 1);
        endRemoveRows();
    }
    else if (nextMiddleCount > previousMiddleCount)
    {
        const int insertStartRow = prefixCount + previousMiddleCount;
        const int insertEndRow = prefixCount + nextMiddleCount - 1;
        beginInsertRows(QModelIndex(), insertStartRow, insertEndRow);
        for (int offset = previousMiddleCount; offset < nextMiddleCount; ++offset)
        {
            m_entries.insert(insertStartRow + (offset - previousMiddleCount), nextEntries.at(prefixCount + offset));
        }
        endInsertRows();
    }

    const int suffixStartRow = std::max(0, nextCount - suffixCount);
    for (int row = suffixStartRow; row < nextCount; ++row)
    {
        if (replaceEntryIfChanged(row, nextEntries.at(row)))
        {
            changedRows.push_back(row);
        }
    }

    emitDataChangedForRows(changedRows);
    if (previousCount != m_entries.size())
    {
        emit countChanged();
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentBlocksModel"),
        QStringLiteral("setBlocks"),
        QStringLiteral("previous=%1 next=%2 prefix=%3 suffix=%4")
            .arg(previousCount)
            .arg(m_entries.size())
            .arg(prefixCount)
            .arg(suffixCount));
    emit blocksChanged();
}

int ContentsStructuredDocumentBlocksModel::count() const noexcept
{
    return rowCount();
}

int ContentsStructuredDocumentBlocksModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_entries.size();
}

QVariant ContentsStructuredDocumentBlocksModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
    {
        return {};
    }

    const Entry& entry = m_entries.at(index.row());
    switch (role)
    {
    case BlockDataRole:
        return entry.blockData;
    case BlockStableKeyRole:
        return entry.stableKey;
    default:
        break;
    }
    return {};
}

QHash<int, QByteArray> ContentsStructuredDocumentBlocksModel::roleNames() const
{
    return {
        {BlockDataRole, QByteArrayLiteral("blockData")},
        {BlockStableKeyRole, QByteArrayLiteral("blockStableKey")},
    };
}

ContentsStructuredDocumentBlocksModel::Entry
ContentsStructuredDocumentBlocksModel::entryFromVariant(const QVariant& value)
{
    const QVariantMap blockData = normalizedBlockMap(value);
    return Entry{
        blockData,
        stableKeyForBlock(blockData),
    };
}

bool ContentsStructuredDocumentBlocksModel::entriesMatchForRetention(
    const Entry& lhs,
    const Entry& rhs)
{
    return lhs.stableKey == rhs.stableKey;
}

QString ContentsStructuredDocumentBlocksModel::stableKeyForBlock(
    const QVariantMap& blockData)
{
    const QString sourceText = blockData.value(QStringLiteral("sourceText")).toString();
    const QString plainText = blockData.value(QStringLiteral("plainText")).toString();
    return QStringLiteral("%1\x1f%2\x1f%3\x1f%4")
        .arg(blockData.value(QStringLiteral("type")).toString().trimmed().toCaseFolded())
        .arg(blockData.value(QStringLiteral("tagName")).toString().trimmed().toCaseFolded())
        .arg(blockData.value(QStringLiteral("semanticTagName")).toString().trimmed().toCaseFolded())
        .arg(sourceText.isEmpty() ? plainText : sourceText);
}

int ContentsStructuredDocumentBlocksModel::commonPrefixCount(
    const QVector<Entry>& nextEntries) const
{
    const int limit = std::min(m_entries.size(), nextEntries.size());
    int prefixCount = 0;
    while (prefixCount < limit
           && entriesMatchForRetention(m_entries.at(prefixCount), nextEntries.at(prefixCount)))
    {
        ++prefixCount;
    }
    return prefixCount;
}

int ContentsStructuredDocumentBlocksModel::commonSuffixCount(
    const QVector<Entry>& nextEntries,
    const int prefixCount) const
{
    const int previousCount = m_entries.size();
    const int nextCount = nextEntries.size();
    const int maximumSuffix = std::min(previousCount, nextCount) - prefixCount;
    int suffixCount = 0;
    while (suffixCount < maximumSuffix)
    {
        const int previousIndex = previousCount - 1 - suffixCount;
        const int nextIndex = nextCount - 1 - suffixCount;
        if (!entriesMatchForRetention(m_entries.at(previousIndex), nextEntries.at(nextIndex)))
        {
            break;
        }
        ++suffixCount;
    }
    return suffixCount;
}

bool ContentsStructuredDocumentBlocksModel::replaceEntryIfChanged(
    const int row,
    const Entry& nextEntry)
{
    if (row < 0 || row >= m_entries.size())
    {
        return false;
    }

    if (m_entries.at(row).blockData == nextEntry.blockData
        && m_entries.at(row).stableKey == nextEntry.stableKey)
    {
        return false;
    }

    m_entries[row] = nextEntry;
    return true;
}

void ContentsStructuredDocumentBlocksModel::emitDataChangedForRows(
    const QVector<int>& rows)
{
    if (rows.isEmpty())
    {
        return;
    }

    QVector<int> sortedRows = rows;
    std::sort(sortedRows.begin(), sortedRows.end());
    sortedRows.erase(std::unique(sortedRows.begin(), sortedRows.end()), sortedRows.end());

    int rangeStart = sortedRows.first();
    int previousRow = rangeStart;
    for (int index = 1; index < sortedRows.size(); ++index)
    {
        const int currentRow = sortedRows.at(index);
        if (currentRow == previousRow + 1)
        {
            previousRow = currentRow;
            continue;
        }

        emit dataChanged(
            this->index(rangeStart, 0),
            this->index(previousRow, 0),
            QVector<int>{BlockDataRole, BlockStableKeyRole});
        rangeStart = currentRow;
        previousRow = currentRow;
    }

    emit dataChanged(
        this->index(rangeStart, 0),
        this->index(previousRow, 0),
        QVector<int>{BlockDataRole, BlockStableKeyRole});
}
