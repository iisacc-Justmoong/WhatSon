#include "SidebarSelectionStore.hpp"

#include "../../file/WhatSonDebugTrace.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"

#include <algorithm>

namespace
{
    QStringList defaultToolbarIconNames()
    {
        return {
            QStringLiteral("nodeslibraryFolder"),
            QStringLiteral("generalprojectStructure"),
            QStringLiteral("bookmarksbookmarksList"),
            QStringLiteral("vcscurrentBranch"),
            QStringLiteral("imageToImage"),
            QStringLiteral("chartBar"),
            QStringLiteral("dataView"),
            QStringLiteral("dataFile")
        };
    }
} // namespace

SidebarSelectionStore::SidebarSelectionStore(
    LibraryHierarchyViewModel* libraryViewModel,
    ProjectsHierarchyViewModel* projectsViewModel,
    BookmarksHierarchyViewModel* bookmarksViewModel,
    TagsHierarchyViewModel* tagsViewModel,
    ResourcesHierarchyViewModel* resourcesViewModel,
    ProgressHierarchyViewModel* progressViewModel,
    EventHierarchyViewModel* eventViewModel,
    PresetHierarchyViewModel* presetViewModel,
    QObject* parent)
    : QObject(parent)
      , m_libraryViewModel(libraryViewModel)
      , m_projectsViewModel(projectsViewModel)
      , m_bookmarksViewModel(bookmarksViewModel)
      , m_tagsViewModel(tagsViewModel)
      , m_resourcesViewModel(resourcesViewModel)
      , m_progressViewModel(progressViewModel)
      , m_eventViewModel(eventViewModel)
      , m_presetViewModel(presetViewModel)
{
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("ctor"),
        QStringLiteral("library=%1 projects=%2 bookmarks=%3 tags=%4 resources=%5 progress=%6 event=%7 preset=%8")
        .arg(m_libraryViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_projectsViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_bookmarksViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_tagsViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_resourcesViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_progressViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_eventViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_presetViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0")));

    if (m_libraryViewModel != nullptr)
    {
        connect(m_libraryViewModel, &LibraryHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kLibrarySectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_projectsViewModel != nullptr)
    {
        connect(m_projectsViewModel, &ProjectsHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kProjectsSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_bookmarksViewModel != nullptr)
    {
        connect(m_bookmarksViewModel, &BookmarksHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kBookmarksSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_tagsViewModel != nullptr)
    {
        connect(m_tagsViewModel, &TagsHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kTagsSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_resourcesViewModel != nullptr)
    {
        connect(m_resourcesViewModel, &ResourcesHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kResourcesSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_progressViewModel != nullptr)
    {
        connect(m_progressViewModel, &ProgressHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kProgressSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_eventViewModel != nullptr)
    {
        connect(m_eventViewModel, &EventHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kEventSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }

    if (m_presetViewModel != nullptr)
    {
        connect(m_presetViewModel, &PresetHierarchyViewModel::selectedIndexChanged, this, [this]()
        {
            if (activeIndex() == kPresetSectionIndex)
            {
                emitSelectionAndCapabilityUpdates();
            }
        });
    }
}

SidebarSelectionStore::~SidebarSelectionStore() = default;

int SidebarSelectionStore::activeIndex() const noexcept
{
    return m_activeIndex;
}

void SidebarSelectionStore::setActiveIndex(int index)
{
    const int clamped = std::clamp(index, 0, kSectionCount - 1);
    if (m_activeIndex == clamped)
    {
        return;
    }

    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("setActiveIndex"),
        QStringLiteral("requested=%1 previous=%2 next=%3").arg(index).arg(m_activeIndex).arg(clamped));

    m_activeIndex = clamped;
    emit activeIndexChanged();
    emit itemModelChanged();
    emit listItemModelChanged();
    emitSelectionAndCapabilityUpdates();
    emit toolbarStateChanged();
}

QAbstractItemModel* SidebarSelectionStore::itemModel() const noexcept
{
    switch (activeIndex())
    {
    case kLibrarySectionIndex:
        return m_libraryViewModel != nullptr ? m_libraryViewModel->itemModel() : nullptr;
    case kProjectsSectionIndex:
        return m_projectsViewModel != nullptr ? m_projectsViewModel->itemModel() : nullptr;
    case kBookmarksSectionIndex:
        return m_bookmarksViewModel != nullptr ? m_bookmarksViewModel->itemModel() : nullptr;
    case kTagsSectionIndex:
        return m_tagsViewModel != nullptr ? m_tagsViewModel->itemModel() : nullptr;
    case kResourcesSectionIndex:
        return m_resourcesViewModel != nullptr ? m_resourcesViewModel->itemModel() : nullptr;
    case kProgressSectionIndex:
        return m_progressViewModel != nullptr ? m_progressViewModel->itemModel() : nullptr;
    case kEventSectionIndex:
        return m_eventViewModel != nullptr ? m_eventViewModel->itemModel() : nullptr;
    case kPresetSectionIndex:
        return m_presetViewModel != nullptr ? m_presetViewModel->itemModel() : nullptr;
    default:
        return nullptr;
    }
}

QAbstractItemModel* SidebarSelectionStore::listItemModel() const noexcept
{
    if (activeIndex() != kLibrarySectionIndex || m_libraryViewModel == nullptr)
    {
        return nullptr;
    }

    return m_libraryViewModel->noteListModel();
}

int SidebarSelectionStore::selectedIndex() const noexcept
{
    switch (activeIndex())
    {
    case kLibrarySectionIndex:
        return m_libraryViewModel != nullptr ? m_libraryViewModel->selectedIndex() : -1;
    case kProjectsSectionIndex:
        return m_projectsViewModel != nullptr ? m_projectsViewModel->selectedIndex() : -1;
    case kBookmarksSectionIndex:
        return m_bookmarksViewModel != nullptr ? m_bookmarksViewModel->selectedIndex() : -1;
    case kTagsSectionIndex:
        return m_tagsViewModel != nullptr ? m_tagsViewModel->selectedIndex() : -1;
    case kResourcesSectionIndex:
        return m_resourcesViewModel != nullptr ? m_resourcesViewModel->selectedIndex() : -1;
    case kProgressSectionIndex:
        return m_progressViewModel != nullptr ? m_progressViewModel->selectedIndex() : -1;
    case kEventSectionIndex:
        return m_eventViewModel != nullptr ? m_eventViewModel->selectedIndex() : -1;
    case kPresetSectionIndex:
        return m_presetViewModel != nullptr ? m_presetViewModel->selectedIndex() : -1;
    default:
        return -1;
    }
}

void SidebarSelectionStore::setSelectedIndex(int index)
{
    switch (activeIndex())
    {
    case kLibrarySectionIndex:
        if (m_libraryViewModel != nullptr)
        {
            m_libraryViewModel->setSelectedIndex(index);
        }
        return;
    case kProjectsSectionIndex:
        if (m_projectsViewModel != nullptr)
        {
            m_projectsViewModel->setSelectedIndex(index);
        }
        return;
    case kBookmarksSectionIndex:
        if (m_bookmarksViewModel != nullptr)
        {
            m_bookmarksViewModel->setSelectedIndex(index);
        }
        return;
    case kTagsSectionIndex:
        if (m_tagsViewModel != nullptr)
        {
            m_tagsViewModel->setSelectedIndex(index);
        }
        return;
    case kResourcesSectionIndex:
        if (m_resourcesViewModel != nullptr)
        {
            m_resourcesViewModel->setSelectedIndex(index);
        }
        return;
    case kProgressSectionIndex:
        if (m_progressViewModel != nullptr)
        {
            m_progressViewModel->setSelectedIndex(index);
        }
        return;
    case kEventSectionIndex:
        if (m_eventViewModel != nullptr)
        {
            m_eventViewModel->setSelectedIndex(index);
        }
        return;
    case kPresetSectionIndex:
        if (m_presetViewModel != nullptr)
        {
            m_presetViewModel->setSelectedIndex(index);
        }
        return;
    default:
        return;
    }
}

bool SidebarSelectionStore::renameEnabled() const noexcept
{
    return activeIndex() == kLibrarySectionIndex;
}

bool SidebarSelectionStore::createFolderEnabled() const noexcept
{
    return activeIndex() == kLibrarySectionIndex;
}

bool SidebarSelectionStore::deleteFolderEnabled() const noexcept
{
    return activeIndex() == kLibrarySectionIndex && selectedIndex() >= 0;
}

QStringList SidebarSelectionStore::toolbarIconNames() const
{
    return defaultToolbarIconNames();
}

QVariantList SidebarSelectionStore::toolbarItems() const
{
    const QStringList iconNames = toolbarIconNames();
    const int currentIndex = activeIndex();

    QVariantList items;
    items.reserve(iconNames.size());
    for (int index = 0; index < iconNames.size(); ++index)
    {
        items.push_back(QVariantMap{
            {"id", index},
            {"iconName", iconNames.at(index)},
            {"selected", index == currentIndex}
        });
    }
    return items;
}

void SidebarSelectionStore::setDepthItems(const QVariantList& depthItems)
{
    if (!renameEnabled() || m_libraryViewModel == nullptr)
    {
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("setDepthItemsIgnored"),
            QStringLiteral("depthCount=%1").arg(depthItems.size()));
        return;
    }

    m_libraryViewModel->setDepthItems(depthItems);
}

QString SidebarSelectionStore::itemLabel(int index) const
{
    if (renameEnabled() && m_libraryViewModel != nullptr)
    {
        return m_libraryViewModel->itemLabel(index);
    }

    QAbstractItemModel* model = itemModel();
    if (model == nullptr || index < 0 || index >= model->rowCount())
    {
        return {};
    }

    int labelRole = Qt::DisplayRole;
    const QHash<int, QByteArray> roles = model->roleNames();
    for (auto it = roles.constBegin(); it != roles.constEnd(); ++it)
    {
        if (it.value() == QByteArrayLiteral("label"))
        {
            labelRole = it.key();
            break;
        }
    }

    return model->index(index, 0).data(labelRole).toString();
}

bool SidebarSelectionStore::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled() || m_libraryViewModel == nullptr)
    {
        return false;
    }

    return m_libraryViewModel->renameItem(index, displayName);
}

void SidebarSelectionStore::createFolder()
{
    if (!createFolderEnabled() || m_libraryViewModel == nullptr)
    {
        return;
    }

    m_libraryViewModel->createFolder();
}

void SidebarSelectionStore::deleteSelectedFolder()
{
    if (!deleteFolderEnabled() || m_libraryViewModel == nullptr)
    {
        return;
    }

    m_libraryViewModel->deleteSelectedFolder();
}

void SidebarSelectionStore::emitSelectionAndCapabilityUpdates()
{
    emit selectedIndexChanged();
    emit capabilitiesChanged();
}
