#include "app/models/sidebar/HierarchyControllerProvider.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QDebug>

HierarchyControllerProvider::HierarchyControllerProvider(QObject* parent)
    : IHierarchyControllerProvider(parent)
{
}

HierarchyControllerProvider::~HierarchyControllerProvider() = default;

void HierarchyControllerProvider::setMappings(QVector<Mapping> mappings)
{
    if (!WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("HierarchyControllerProvider::setMappings")))
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

QVector<HierarchyControllerProvider::Mapping> HierarchyControllerProvider::mappings() const
{
    return exportedMappings(m_mappings);
}

IHierarchyController* HierarchyControllerProvider::hierarchyController(int hierarchyIndex) const
{
    const int offset = mappingOffsetForIndex(hierarchyIndex);
    if (offset < 0 || offset >= static_cast<int>(m_mappings.size()))
    {
        return nullptr;
    }

    return m_mappings[static_cast<std::size_t>(offset)].data();
}

QObject* HierarchyControllerProvider::noteListModel(int hierarchyIndex) const
{
    IHierarchyController* controller = hierarchyController(hierarchyIndex);
    return controller ? controller->hierarchyNoteListModel() : nullptr;
}

int HierarchyControllerProvider::mappingOffsetForIndex(int hierarchyIndex) noexcept
{
    const int normalizedIndex = WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex);
    return normalizedIndex - WhatSon::Sidebar::kHierarchyMinIndex;
}

std::array<QPointer<IHierarchyController>, HierarchyControllerProvider::kMappingCount>
HierarchyControllerProvider::normalizedMappings(const QVector<Mapping>& mappings)
{
    std::array<QPointer<IHierarchyController>, kMappingCount> normalized;
    normalized.fill(nullptr);

    for (const Mapping& mapping : mappings)
    {
        const int offset = mappingOffsetForIndex(mapping.hierarchyIndex);
        if (offset < 0 || offset >= static_cast<int>(normalized.size()))
        {
            continue;
        }

        normalized[static_cast<std::size_t>(offset)] = mapping.controller;
    }

    return normalized;
}

QVector<HierarchyControllerProvider::Mapping> HierarchyControllerProvider::exportedMappings(
    const std::array<QPointer<IHierarchyController>, kMappingCount>& mappings)
{
    QVector<Mapping> exported;
    exported.reserve(static_cast<qsizetype>(mappings.size()));

    for (int offset = 0; offset < static_cast<int>(mappings.size()); ++offset)
    {
        IHierarchyController* controller = mappings[static_cast<std::size_t>(offset)].data();
        if (controller == nullptr)
        {
            continue;
        }

        Mapping mapping;
        mapping.hierarchyIndex = WhatSon::Sidebar::kHierarchyMinIndex + offset;
        mapping.controller = controller;
        exported.push_back(mapping);
    }

    return exported;
}
