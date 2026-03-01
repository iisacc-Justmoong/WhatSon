#include "BookmarksHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/bookmarks/WhatSonBookmarksHierarchyParser.hpp"
#include "file/hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

namespace
{
    constexpr auto kScope = "bookmarks.viewmodel";
}

BookmarksHierarchyViewModel::BookmarksHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::trace(QString::fromLatin1(kScope), QStringLiteral("ctor"));
    setBookmarkIds({});
}

BookmarksHierarchyViewModel::~BookmarksHierarchyViewModel() = default;

FlatHierarchyModel* BookmarksHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int BookmarksHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

void BookmarksHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_items.size());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("setSelectedIndex"),
        QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void BookmarksHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Bookmark"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList BookmarksHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

QString BookmarksHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool BookmarksHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled())
    {
        return false;
    }
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }
    if (WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(index)))
    {
        return false;
    }

    if (!WhatSon::Hierarchy::Support::renameFlatItem(&m_items, index, displayName))
    {
        return false;
    }

    syncDomainStoreFromItems();
    syncModel();
    return true;
}

void BookmarksHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::Support::createFlatFolder(
        &m_items, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        return;
    }

    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(insertIndex);
}

void BookmarksHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int nextSelectedIndex = WhatSon::Hierarchy::Support::deleteFlatSubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void BookmarksHierarchyViewModel::setBookmarkIds(QStringList bookmarkIds)
{
    m_bookmarkIds = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(bookmarkIds));
    m_store.setBookmarkIds(m_bookmarkIds);
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Bookmarks"),
        m_bookmarkIds,
        QStringLiteral("Bookmark"));
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
}

QStringList BookmarksHierarchyViewModel::bookmarkIds() const
{
    return m_bookmarkIds;
}

bool BookmarksHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool BookmarksHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool BookmarksHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool BookmarksHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::Support::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        return false;
    }

    QStringList aggregated;
    bool fileFound = false;

    WhatSonBookmarksHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Bookmarks.wsbookmarks"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        fileFound = true;

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::Support::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        QString parseError;
        if (!parser.parse(rawText, &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        for (const QString& value : m_store.bookmarkIds())
        {
            aggregated.push_back(value);
        }
    }

    setBookmarkIds(aggregated);

    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("loadFromWshub"),
        QStringLiteral("path=%1 fileFound=%2 count=%3")
        .arg(wshubPath)
        .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_bookmarkIds.size()));
    return true;
}

void BookmarksHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}

void BookmarksHierarchyViewModel::syncDomainStoreFromItems()
{
    m_bookmarkIds = WhatSon::Hierarchy::Support::extractDomainLabelsFromItems(m_items);
    m_store.setBookmarkIds(m_bookmarkIds);
}
