#include "app/viewmodel/sidebar/HierarchyViewModelProvider.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QDebug>

HierarchyViewModelProvider::HierarchyViewModelProvider(QObject* parent)
    : IHierarchyViewModelProvider(parent)
{
}

HierarchyViewModelProvider::~HierarchyViewModelProvider() = default;

void HierarchyViewModelProvider::setMappings(QVector<Mapping> mappings)
{
    if (!WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("HierarchyViewModelProvider::setMappings")))
    {
        return;
    }

    const auto normalized = normalizedMappings(mappings);
    if (m_mappings == normalized)
    {
        return;
    }

    m_mappings = normalized;
    emit mappingsChanged();
}

QVector<HierarchyViewModelProvider::Mapping> HierarchyViewModelProvider::mappings() const
{
    return exportedMappings(m_mappings);
}

IHierarchyViewModel* HierarchyViewModelProvider::hierarchyViewModel(int hierarchyIndex) const
{
    const int offset = mappingOffsetForIndex(hierarchyIndex);
    if (offset < 0 || offset >= static_cast<int>(m_mappings.size()))
    {
        return nullptr;
    }

    return m_mappings[static_cast<std::size_t>(offset)].data();
}

QObject* HierarchyViewModelProvider::noteListModel(int hierarchyIndex) const
{
    IHierarchyViewModel* viewModel = hierarchyViewModel(hierarchyIndex);
    return viewModel ? viewModel->hierarchyNoteListModel() : nullptr;
}

int HierarchyViewModelProvider::mappingOffsetForIndex(int hierarchyIndex) noexcept
{
    const int normalizedIndex = WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex);
    return normalizedIndex - WhatSon::Sidebar::kHierarchyMinIndex;
}

std::array<QPointer<IHierarchyViewModel>, HierarchyViewModelProvider::kMappingCount>
HierarchyViewModelProvider::normalizedMappings(const QVector<Mapping>& mappings)
{
    std::array<QPointer<IHierarchyViewModel>, kMappingCount> normalized;
    normalized.fill(nullptr);

    for (const Mapping& mapping : mappings)
    {
        const int offset = mappingOffsetForIndex(mapping.hierarchyIndex);
        if (offset < 0 || offset >= static_cast<int>(normalized.size()))
        {
            continue;
        }

        normalized[static_cast<std::size_t>(offset)] = mapping.viewModel;
    }

    return normalized;
}

QVector<HierarchyViewModelProvider::Mapping> HierarchyViewModelProvider::exportedMappings(
    const std::array<QPointer<IHierarchyViewModel>, kMappingCount>& mappings)
{
    QVector<Mapping> exported;
    exported.reserve(static_cast<qsizetype>(mappings.size()));

    for (int offset = 0; offset < static_cast<int>(mappings.size()); ++offset)
    {
        IHierarchyViewModel* viewModel = mappings[static_cast<std::size_t>(offset)].data();
        if (viewModel == nullptr)
        {
            continue;
        }

        Mapping mapping;
        mapping.hierarchyIndex = WhatSon::Sidebar::kHierarchyMinIndex + offset;
        mapping.viewModel = viewModel;
        exported.push_back(mapping);
    }

    return exported;
}
