#include "LibraryNoteListModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <utility>

LibraryNoteListModel::LibraryNoteListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("library.notelist.model"), QStringLiteral("ctor"));
}

int LibraryNoteListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant LibraryNoteListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const LibraryNoteListItem& item = m_items.at(index.row());
    switch (role)
    {
    case NoteIdRole:
        return item.noteId;
    case TitleTextRole:
        return item.titleText;
    case SummaryTextRole:
        return item.summaryText;
    case FoldersTextRole:
        return item.foldersText;
    case BookmarkedRole:
        return item.bookmarked;
    case BookmarkColorHexRole:
        return item.bookmarkColorHex;
    case HighlightedRole:
        return item.highlighted;
    default:
        return {};
    }
}

QHash<int, QByteArray> LibraryNoteListModel::roleNames() const
{
    return {
        {NoteIdRole, "noteId"},
        {TitleTextRole, "titleText"},
        {SummaryTextRole, "summaryText"},
        {FoldersTextRole, "foldersText"},
        {BookmarkedRole, "bookmarked"},
        {BookmarkColorHexRole, "bookmarkColorHex"},
        {HighlightedRole, "highlighted"}
    };
}

void LibraryNoteListModel::setItems(QVector<LibraryNoteListItem> items)
{
    WhatSon::Debug::trace(
        QStringLiteral("library.notelist.model"),
        QStringLiteral("setItems"),
        QStringLiteral("count=%1").arg(items.size()));
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

const QVector<LibraryNoteListItem>& LibraryNoteListModel::items() const noexcept
{
    return m_items;
}
