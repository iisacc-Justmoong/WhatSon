#include "HierarchyViewModelProvider.hpp"

#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"

#include <QMetaType>
#include <QVariant>

HierarchyViewModelProvider::HierarchyViewModelProvider(QObject* parent)
    : IHierarchyViewModelProvider(parent)
{
}

HierarchyViewModelProvider::~HierarchyViewModelProvider() = default;

void HierarchyViewModelProvider::setTargets(Targets targets)
{
    if (sameTargets(m_targets, targets))
    {
        return;
    }

    m_targets = targets;
    emit mappingsChanged();
}

HierarchyViewModelProvider::Targets HierarchyViewModelProvider::targets() const noexcept
{
    return m_targets;
}

QObject* HierarchyViewModelProvider::hierarchyViewModel(int hierarchyIndex) const
{
    const int normalizedIndex = WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex);
    switch (normalizedIndex)
    {
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library):
        return m_targets.libraryViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Projects):
        return m_targets.projectsViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Bookmarks):
        return m_targets.bookmarksViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags):
        return m_targets.tagsViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources):
        return m_targets.resourcesViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Progress):
        return m_targets.progressViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Event):
        return m_targets.eventViewModel;
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Preset):
        return m_targets.presetViewModel;
    default:
        break;
    }
    return nullptr;
}

QObject* HierarchyViewModelProvider::noteListModel(int hierarchyIndex) const
{
    return noteListModelFromViewModel(hierarchyViewModel(hierarchyIndex));
}

bool HierarchyViewModelProvider::sameTargets(const Targets& lhs, const Targets& rhs) noexcept
{
    return lhs.libraryViewModel == rhs.libraryViewModel
        && lhs.projectsViewModel == rhs.projectsViewModel
        && lhs.bookmarksViewModel == rhs.bookmarksViewModel
        && lhs.tagsViewModel == rhs.tagsViewModel
        && lhs.resourcesViewModel == rhs.resourcesViewModel
        && lhs.progressViewModel == rhs.progressViewModel
        && lhs.eventViewModel == rhs.eventViewModel
        && lhs.presetViewModel == rhs.presetViewModel;
}

QObject* HierarchyViewModelProvider::noteListModelFromViewModel(QObject* viewModel)
{
    if (viewModel == nullptr)
    {
        return nullptr;
    }

    const QVariant noteListProperty = viewModel->property("noteListModel");
    if (!noteListProperty.isValid() || noteListProperty.isNull())
    {
        return nullptr;
    }

    if (noteListProperty.metaType().id() == QMetaType::QObjectStar)
    {
        return noteListProperty.value<QObject*>();
    }

    QVariant converted = noteListProperty;
    if (converted.convert(QMetaType::fromType<QObject*>()))
    {
        return converted.value<QObject*>();
    }

    return nullptr;
}
