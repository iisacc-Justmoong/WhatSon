#include "SidebarSelectionStore.hpp"

#include "hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "hierarchy/tags/TagsHierarchyViewModel.hpp"
#include "sidebar/SidebarHierarchyStore.hpp"

#include <algorithm>

SidebarSelectionStore::SidebarSelectionStore(
    SidebarHierarchyStore* sidebarStore,
    LibraryHierarchyViewModel* libraryViewModel,
    TagsHierarchyViewModel* tagsViewModel,
    QObject* parent)
    : QObject(parent)
      , m_sidebarStore(sidebarStore)
      , m_libraryViewModel(libraryViewModel)
      , m_tagsViewModel(tagsViewModel)
{
    if (m_sidebarStore != nullptr)
    {
        connect(m_sidebarStore, &SidebarHierarchyStore::activeIndexChanged, this, [this]()
        {
            emit activeIndexChanged();
            emit itemModelChanged();
            emit selectedIndexChanged();
            emit capabilitiesChanged();
            emit toolbarStateChanged();
        });
    }

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
}

SidebarSelectionStore::~SidebarSelectionStore() = default;

int SidebarSelectionStore::activeIndex() const noexcept
{
    if (m_sidebarStore == nullptr)
    {
        return 0;
    }
    return m_sidebarStore->activeIndex();
}

void SidebarSelectionStore::setActiveIndex(int index)
{
    if (m_sidebarStore == nullptr)
    {
        return;
    }
    m_sidebarStore->setActiveIndex(index);
}

QAbstractItemModel* SidebarSelectionStore::itemModel() const noexcept
{
    const int currentIndex = activeIndex();
    if (currentIndex == kLibrarySectionIndex && m_libraryViewModel != nullptr)
    {
        return m_libraryViewModel->itemModel();
    }
    if (currentIndex == kTagsSectionIndex && m_tagsViewModel != nullptr)
    {
        return m_tagsViewModel->itemModel();
    }
    if (m_sidebarStore != nullptr)
    {
        return m_sidebarStore->itemModel();
    }
    return nullptr;
}

int SidebarSelectionStore::selectedIndex() const noexcept
{
    const int currentIndex = activeIndex();
    if (currentIndex == kLibrarySectionIndex && m_libraryViewModel != nullptr)
    {
        return m_libraryViewModel->selectedIndex();
    }
    if (currentIndex == kTagsSectionIndex && m_tagsViewModel != nullptr)
    {
        return m_tagsViewModel->selectedIndex();
    }
    return m_genericSelectedIndexBySection.value(currentIndex, -1);
}

void SidebarSelectionStore::setSelectedIndex(int index)
{
    const int currentIndex = activeIndex();
    if (currentIndex == kLibrarySectionIndex && m_libraryViewModel != nullptr)
    {
        m_libraryViewModel->setSelectedIndex(index);
        return;
    }
    if (currentIndex == kTagsSectionIndex && m_tagsViewModel != nullptr)
    {
        m_tagsViewModel->setSelectedIndex(index);
        return;
    }

    QAbstractItemModel* model = itemModel();
    const int rowCount = model == nullptr ? 0 : model->rowCount();
    int clamped = -1;
    if (rowCount > 0)
    {
        clamped = std::clamp(index, -1, rowCount - 1);
    }

    if (m_genericSelectedIndexBySection.value(currentIndex, -1) == clamped)
    {
        return;
    }

    m_genericSelectedIndexBySection.insert(currentIndex, clamped);
    emitSelectionAndCapabilityUpdates();
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
    if (m_sidebarStore == nullptr)
    {
        return {};
    }
    return m_sidebarStore->toolbarIconNames();
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
