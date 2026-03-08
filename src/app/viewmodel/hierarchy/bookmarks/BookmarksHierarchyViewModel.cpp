#include "BookmarksHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"

#include <QDateTime>
#include <QFileInfo>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kScope = "bookmarks.viewmodel";
    constexpr int kMaxNoteListSummaryLines = 5;

    QString truncateToMaxLines(const QString& value, int maxLines)
    {
        if (maxLines <= 0)
        {
            return {};
        }

        const QStringList lines = value.split(QLatin1Char('\n'));
        if (lines.size() <= maxLines)
        {
            return value;
        }

        QStringList truncated;
        truncated.reserve(maxLines);
        for (int index = 0; index < maxLines; ++index)
        {
            truncated.push_back(lines.at(index));
        }
        return truncated.join(QLatin1Char('\n'));
    }

    QString bookmarkLabelText(const LibraryNoteRecord& note)
    {
        QString primary = note.bodyFirstLine.trimmed();
        if (!primary.isEmpty())
        {
            return primary;
        }

        primary = note.noteId.trimmed();
        if (!primary.isEmpty())
        {
            return primary;
        }

        if (!note.noteDirectoryPath.isEmpty())
        {
            primary = QFileInfo(note.noteDirectoryPath).completeBaseName().trimmed();
            if (!primary.isEmpty())
            {
                return primary;
            }
        }

        return {};
    }

    QString bookmarkPrimaryText(const LibraryNoteRecord& note)
    {
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }
        return bookmarkLabelText(note);
    }

    QString bookmarkSearchableText(const LibraryNoteRecord& note)
    {
        QStringList parts;

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            parts.push_back(noteId);
        }

        const QString firstLine = note.bodyFirstLine.trimmed();
        if (!firstLine.isEmpty())
        {
            parts.push_back(firstLine);
        }

        const QString bodyPlainText = note.bodyPlainText.trimmed();
        if (!bodyPlainText.isEmpty())
        {
            parts.push_back(bodyPlainText);
        }

        for (const QString& folder : note.folders)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }

        for (const QString& tag : note.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }

        return parts.join(QLatin1Char('\n'));
    }

    QStringList bookmarkListFolders(const LibraryNoteRecord& note)
    {
        QStringList folders;
        folders.reserve(note.folders.size());
        for (const QString& folder : note.folders)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                folders.push_back(trimmed);
            }
        }
        folders.removeDuplicates();
        return folders;
    }

    QStringList bookmarkListTags(const LibraryNoteRecord& note)
    {
        QStringList tags;
        tags.reserve(note.tags.size());
        for (const QString& tag : note.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                tags.push_back(trimmed);
            }
        }
        tags.removeDuplicates();
        return tags;
    }

    QDate parseBookmarkDate(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        const QList<QString> formats{
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-dd hh:mm:ss"),
            QStringLiteral("yyyy/MM/dd hh:mm:ss"),
            QStringLiteral("yyyy-MM-dd")
        };

        for (const QString& format : formats)
        {
            const QDateTime dateTime = QDateTime::fromString(trimmed, format);
            if (dateTime.isValid())
            {
                return dateTime.date();
            }

            const QDate date = QDate::fromString(trimmed, format);
            if (date.isValid())
            {
                return date;
            }
        }

        const QDateTime isoDateTime = QDateTime::fromString(trimmed, Qt::ISODate);
        if (isoDateTime.isValid())
        {
            return isoDateTime.date();
        }

        const QDateTime isoDateTimeWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoDateTimeWithMs.isValid())
        {
            return isoDateTimeWithMs.date();
        }

        return {};
    }

    QString bookmarkDisplayDate(const LibraryNoteRecord& note)
    {
        QDate date = parseBookmarkDate(note.lastModifiedAt);
        if (!date.isValid())
        {
            date = parseBookmarkDate(note.createdAt);
        }
        if (!date.isValid())
        {
            return {};
        }
        return date.toString(QStringLiteral("yyyy-MM-dd"));
    }

    QString bookmarkColorHexForNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }

    QString colorHexForLabel(const QString& label)
    {
        const QString normalized = label.trimmed().toCaseFolded();
        for (const auto& colorDef : WhatSon::Bookmarks::kBookmarkColorDefinitions)
        {
            if (normalized == QString::fromLatin1(colorDef.name))
            {
                return QString::fromLatin1(colorDef.hex);
            }
        }
        return {};
    }

    QVector<BookmarksHierarchyItem> buildColorFolderItems()
    {
        QVector<BookmarksHierarchyItem> items;
        items.reserve(static_cast<int>(WhatSon::Bookmarks::kBookmarkColorDefinitions.size()));

        for (const auto& colorDef : WhatSon::Bookmarks::kBookmarkColorDefinitions)
        {
            BookmarksHierarchyItem item;
            item.depth = 0;
            item.accent = false;
            item.expanded = false;
            item.label = QString::fromLatin1(colorDef.name);
            item.showChevron = false;
            items.push_back(std::move(item));
        }

        return items;
    }
} // namespace

BookmarksHierarchyViewModel::BookmarksHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &BookmarksHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    QObject::connect(
        &m_noteListModel,
        &LibraryNoteListModel::itemCountChanged,
        this,
        [this](int)
        {
            updateNoteItemCount();
        });

    rebuildColorFolders();
    refreshNoteListForSelection();
}

BookmarksHierarchyViewModel::~BookmarksHierarchyViewModel() = default;

BookmarksHierarchyModel* BookmarksHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* BookmarksHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

int BookmarksHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int BookmarksHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

int BookmarksHierarchyViewModel::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool BookmarksHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString BookmarksHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void BookmarksHierarchyViewModel::setSelectedIndex(int index)
{
    const int maxIndex = m_itemModel.rowCount() - 1;
    int clamped = index;
    if (maxIndex < 0)
    {
        clamped = -1;
    }
    else
    {
        clamped = std::clamp(index, -1, maxIndex);
    }

    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void BookmarksHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    Q_UNUSED(depthItems);
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1 noteCount=%2").arg(m_items.size()).arg(
                                  m_noteListModel.rowCount()));
}

QVariantList BookmarksHierarchyViewModel::depthItems() const
{
    QVariantList serialized;
    serialized.reserve(m_items.size());

    for (const BookmarksHierarchyItem& item : m_items)
    {
        serialized.push_back(QVariantMap{
            {QStringLiteral("label"), item.label},
            {QStringLiteral("depth"), item.depth},
            {QStringLiteral("accent"), item.accent},
            {QStringLiteral("expanded"), item.expanded},
            {QStringLiteral("showChevron"), item.showChevron}
        });
    }

    return serialized;
}

QString BookmarksHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool BookmarksHierarchyViewModel::canRenameItem(int index) const
{
    Q_UNUSED(index);
    return false;
}

bool BookmarksHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    Q_UNUSED(index);
    Q_UNUSED(displayName);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
    return false;
}

void BookmarksHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
}

void BookmarksHierarchyViewModel::deleteSelectedFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
}

bool BookmarksHierarchyViewModel::renameEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::createFolderEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::viewOptionsEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    LibraryAll libraryAll;
    QString indexError;
    if (!libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.index"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, indexError));
        updateLoadState(false, indexError);
        return false;
    }

    const QVector<LibraryNoteRecord>& notes = libraryAll.notes();

    QVector<LibraryNoteListItem> bookmarkListItems;
    bookmarkListItems.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        if (!note.bookmarked)
        {
            continue;
        }

        LibraryNoteListItem item;
        item.id = note.noteId.trimmed();
        item.primaryText = bookmarkPrimaryText(note);
        item.searchableText = bookmarkSearchableText(note);
        item.bodyText = note.bodyPlainText.trimmed();
        item.displayDate = bookmarkDisplayDate(note);
        item.folders = bookmarkListFolders(note);
        item.tags = bookmarkListTags(note);
        item.bookmarked = true;
        item.bookmarkColor = bookmarkColorHexForNote(note);
        bookmarkListItems.push_back(std::move(item));
    }

    m_allBookmarkedNotes = std::move(bookmarkListItems);
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub"),
                              QStringLiteral("path=%1 source=wsnhead count=%2")
                              .arg(wshubPath)
                              .arg(m_allBookmarkedNotes.size()));
    updateLoadState(true);
    return true;
}

void BookmarksHierarchyViewModel::applyRuntimeSnapshot(
    QVector<LibraryNoteRecord> bookmarkedNotes,
    bool loadSucceeded,
    QString errorMessage)
{
    if (!loadSucceeded)
    {
        m_allBookmarkedNotes.clear();
        rebuildColorFolders();
        setSelectedIndex(-1);
        refreshNoteListForSelection();
        updateLoadState(false, errorMessage);
        return;
    }

    QVector<LibraryNoteListItem> bookmarkListItems;
    bookmarkListItems.reserve(bookmarkedNotes.size());

    for (const LibraryNoteRecord& note : bookmarkedNotes)
    {
        LibraryNoteListItem item;
        item.id = note.noteId.trimmed();
        item.primaryText = bookmarkPrimaryText(note);
        item.searchableText = bookmarkSearchableText(note);
        item.bodyText = note.bodyPlainText.trimmed();
        item.displayDate = bookmarkDisplayDate(note);
        item.folders = bookmarkListFolders(note);
        item.tags = bookmarkListTags(note);
        item.bookmarked = true;
        item.bookmarkColor = bookmarkColorHexForNote(note);
        bookmarkListItems.push_back(std::move(item));
    }

    m_allBookmarkedNotes = std::move(bookmarkListItems);
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();
    updateLoadState(true);
}

void BookmarksHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void BookmarksHierarchyViewModel::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }
    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void BookmarksHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    const QString normalizedError = succeeded ? QString() : errorMessage;
    const bool shouldEmit = (m_loadSucceeded != succeeded) || (m_lastLoadError != normalizedError);
    m_loadSucceeded = succeeded;
    m_lastLoadError = normalizedError;
    if (shouldEmit)
    {
        emit loadStateChanged();
    }
}

void BookmarksHierarchyViewModel::rebuildColorFolders()
{
    m_items = buildColorFolderItems();
    syncModel();
}

void BookmarksHierarchyViewModel::refreshNoteListForSelection()
{
    const QString selectedColorHex = colorHexForLabel(selectedColorLabel());
    if (selectedColorHex.isEmpty())
    {
        m_noteListModel.setItems(m_allBookmarkedNotes);
        updateNoteItemCount();
        return;
    }

    QVector<LibraryNoteListItem> filtered;
    filtered.reserve(m_allBookmarkedNotes.size());
    for (const LibraryNoteListItem& item : m_allBookmarkedNotes)
    {
        if (item.bookmarkColor.compare(selectedColorHex, Qt::CaseInsensitive) != 0)
        {
            continue;
        }
        filtered.push_back(item);
    }

    m_noteListModel.setItems(filtered);
    updateNoteItemCount();
}

QString BookmarksHierarchyViewModel::selectedColorLabel() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return {};
    }

    return m_items.at(m_selectedIndex).label.trimmed();
}

void BookmarksHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
}
