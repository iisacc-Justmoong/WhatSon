#include "SidebarHierarchyStore.hpp"

#include "HierarchySectionModel.hpp"

#include <algorithm>

namespace
{
    std::vector<std::unique_ptr<HierarchySectionModel>> createDefaultSections()
    {
        std::vector<std::unique_ptr<HierarchySectionModel>> sections;
        sections.reserve(8);
        sections.emplace_back(std::make_unique<LibraryModel>());
        sections.emplace_back(std::make_unique<ProjectsModel>());
        sections.emplace_back(std::make_unique<BookmarksModel>());
        sections.emplace_back(std::make_unique<TagsModel>());
        sections.emplace_back(std::make_unique<ResourcesModel>());
        sections.emplace_back(std::make_unique<ProgressModel>());
        sections.emplace_back(std::make_unique<EventModel>());
        sections.emplace_back(std::make_unique<PresetModel>());
        return sections;
    }
} // namespace

SidebarHierarchyStore::SidebarHierarchyStore(QObject* parent)
    : QObject(parent)
      , m_sections(createDefaultSections())
{
    syncActiveSection();
}

SidebarHierarchyStore::~SidebarHierarchyStore() = default;

int SidebarHierarchyStore::activeIndex() const noexcept
{
    return m_activeIndex;
}

void SidebarHierarchyStore::setActiveIndex(int index)
{
    if (m_sections.empty())
    {
        return;
    }

    const int maxIndex = static_cast<int>(m_sections.size()) - 1;
    const int clampedIndex = std::clamp(index, 0, maxIndex);
    if (m_activeIndex == clampedIndex)
    {
        return;
    }

    m_activeIndex = clampedIndex;
    syncActiveSection();
    emit activeIndexChanged();
}

HierarchyItemListModel* SidebarHierarchyStore::itemModel() noexcept
{
    return &m_itemModel;
}

QStringList SidebarHierarchyStore::sectionNames() const
{
    QStringList names;
    names.reserve(static_cast<qsizetype>(m_sections.size()));
    for (const std::unique_ptr<HierarchySectionModel>& section : m_sections)
    {
        names.push_back(section->sectionName());
    }
    return names;
}

QStringList SidebarHierarchyStore::toolbarIconNames() const
{
    QStringList iconNames;
    iconNames.reserve(static_cast<qsizetype>(m_sections.size()));
    for (const std::unique_ptr<HierarchySectionModel>& section : m_sections)
    {
        iconNames.push_back(section->toolbarIconName());
    }
    return iconNames;
}

int SidebarHierarchyStore::sectionCount() const noexcept
{
    return static_cast<int>(m_sections.size());
}

void SidebarHierarchyStore::syncActiveSection()
{
    if (m_sections.empty())
    {
        m_itemModel.setItems({});
        return;
    }

    const int maxIndex = static_cast<int>(m_sections.size()) - 1;
    const int safeIndex = std::clamp(m_activeIndex, 0, maxIndex);
    m_itemModel.setItems(m_sections.at(static_cast<std::size_t>(safeIndex))->items());
}
