#include "app/store/sidebar/SidebarSelectionStore.hpp"

#include "app/viewmodel/sidebar/HierarchySidebarDomain.hpp"

SidebarSelectionStore::SidebarSelectionStore(QObject* parent)
    : ISidebarSelectionStore(parent)
{
    m_selectedHierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
}

SidebarSelectionStore::~SidebarSelectionStore() = default;

int SidebarSelectionStore::selectedHierarchyIndex() const noexcept
{
    return m_selectedHierarchyIndex;
}

void SidebarSelectionStore::setSelectedHierarchyIndex(int index)
{
    const int normalizedIndex = WhatSon::Sidebar::normalizeHierarchyIndex(index);
    if (m_selectedHierarchyIndex == normalizedIndex)
    {
        return;
    }

    m_selectedHierarchyIndex = normalizedIndex;
    emit selectedHierarchyIndexChanged();
}
