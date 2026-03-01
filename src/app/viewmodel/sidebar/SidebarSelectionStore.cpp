#include "SidebarSelectionStore.hpp"

#include "../../file/WhatSonDebugTrace.hpp"
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
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("ctor"),
        QStringLiteral("hasSidebarStore=%1 hasLibraryViewModel=%2 hasTagsViewModel=%3")
        .arg(m_sidebarStore != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_libraryViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(m_tagsViewModel != nullptr ? QStringLiteral("1") : QStringLiteral("0")));
    if (m_sidebarStore != nullptr)
    {
        connect(m_sidebarStore, &SidebarHierarchyStore::activeIndexChanged, this, [this]()
        {
            WhatSon::Debug::trace(
                QStringLiteral("sidebar.selection"),
                QStringLiteral("activeIndexChangedSignal"),
                QStringLiteral("active=%1").arg(activeIndex()));
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
                WhatSon::Debug::trace(
                    QStringLiteral("sidebar.selection"),
                    QStringLiteral("librarySelectedIndexChanged"),
                    QStringLiteral("selected=%1").arg(selectedIndex()));
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
                WhatSon::Debug::trace(
                    QStringLiteral("sidebar.selection"),
                    QStringLiteral("tagsSelectedIndexChanged"),
                    QStringLiteral("selected=%1").arg(selectedIndex()));
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
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("setActiveIndexIgnored"),
            QStringLiteral("reason=noSidebarStore requested=%1").arg(index));
        return;
    }
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("setActiveIndex"),
        QStringLiteral("requested=%1 previous=%2").arg(index).arg(m_sidebarStore->activeIndex()));
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
        return m_sidebarStore->itemModelForSection(currentIndex);
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
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("setSelectedIndexRequested"),
        QStringLiteral("section=%1 requested=%2").arg(currentIndex).arg(index));
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
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("setSelectedIndexNoop"),
            QStringLiteral("section=%1 clamped=%2").arg(currentIndex).arg(clamped));
        return;
    }

    m_genericSelectedIndexBySection.insert(currentIndex, clamped);
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("setSelectedIndexApplied"),
        QStringLiteral("section=%1 rowCount=%2 selected=%3")
        .arg(currentIndex)
        .arg(rowCount)
        .arg(clamped));
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
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("setDepthItemsIgnored"),
            QStringLiteral("reason=capabilityOrViewModel depthCount=%1")
            .arg(depthItems.size()));
        return;
    }
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("setDepthItems"),
        QStringLiteral("depthCount=%1").arg(depthItems.size()));
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
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("renameItemIgnored"),
            QStringLiteral("index=%1 name=%2").arg(index).arg(displayName));
        return false;
    }
    const bool renamed = m_libraryViewModel->renameItem(index, displayName);
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("renameItem"),
        QStringLiteral("index=%1 name=%2 renamed=%3")
        .arg(index)
        .arg(displayName)
        .arg(renamed ? QStringLiteral("1") : QStringLiteral("0")));
    return renamed;
}

void SidebarSelectionStore::createFolder()
{
    if (!createFolderEnabled() || m_libraryViewModel == nullptr)
    {
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("createFolderIgnored"));
        return;
    }
    WhatSon::Debug::trace(QStringLiteral("sidebar.selection"), QStringLiteral("createFolder"));
    m_libraryViewModel->createFolder();
}

void SidebarSelectionStore::deleteSelectedFolder()
{
    if (!deleteFolderEnabled() || m_libraryViewModel == nullptr)
    {
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.selection"),
            QStringLiteral("deleteSelectedFolderIgnored"),
            QStringLiteral("selected=%1").arg(selectedIndex()));
        return;
    }
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("deleteSelectedFolder"),
        QStringLiteral("selected=%1").arg(selectedIndex()));
    m_libraryViewModel->deleteSelectedFolder();
}

void SidebarSelectionStore::emitSelectionAndCapabilityUpdates()
{
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.selection"),
        QStringLiteral("emitSelectionAndCapabilityUpdates"),
        QStringLiteral("active=%1 selected=%2 renameEnabled=%3 createEnabled=%4 deleteEnabled=%5")
        .arg(activeIndex())
        .arg(selectedIndex())
        .arg(renameEnabled() ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(createFolderEnabled() ? QStringLiteral("1") : QStringLiteral("0"))
        .arg(deleteFolderEnabled() ? QStringLiteral("1") : QStringLiteral("0")));
    emit selectedIndexChanged();
    emit capabilitiesChanged();
}
