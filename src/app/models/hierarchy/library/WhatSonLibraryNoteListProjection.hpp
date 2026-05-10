#pragma once

#include "app/models/calendar/ISystemCalendarStore.hpp"
#include "app/models/hierarchy/library/LibraryHierarchyModel.hpp"
#include "app/models/hierarchy/library/LibraryNoteListModel.hpp"
#include "app/models/hierarchy/library/LibraryNoteRecord.hpp"

#include <QHash>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonLibraryNoteListProjection final
{
public:
    void setSystemCalendarStore(ISystemCalendarStore* store);
    ISystemCalendarStore* systemCalendarStore() const noexcept;

    void invalidate() const;
    void invalidateForNoteId(const QString& noteId) const;

    QHash<QString, int> folderNoteCountByFolderUuid(
        const QVector<LibraryHierarchyItem>& hierarchyItems,
        const QVector<LibraryNoteRecord>& notes,
        bool foldersHierarchyLoaded) const;

    QVector<LibraryNoteListItem> buildNoteListItems(
        const QVector<LibraryHierarchyItem>& hierarchyItems,
        const QVector<LibraryNoteRecord>& notes,
        bool foldersHierarchyLoaded) const;

    QVector<LibraryNoteListItem> buildFolderScopedNoteListItems(
        const QVector<LibraryHierarchyItem>& hierarchyItems,
        const QVector<LibraryNoteRecord>& notes,
        const QString& selectedFolderUuid) const;

private:
    LibraryNoteListItem buildNoteListItem(
        const LibraryNoteRecord& note,
        const QStringList& folderLabels) const;

    QPointer<ISystemCalendarStore> m_systemCalendarStore;
    mutable QHash<QString, LibraryNoteListItem> m_noteListItemCache;
    mutable bool m_noteListItemCacheUsesFoldersHierarchy = false;
};
