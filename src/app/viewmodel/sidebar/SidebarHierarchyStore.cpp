#include "SidebarHierarchyStore.hpp"

#include "HierarchySectionModel.hpp"
#include "../../file/WhatSonDebugTrace.hpp"

#include <algorithm>
#include <array>

namespace
{
    QString sectionObjectNameForIndex(int index)
    {
        static constexpr std::array<const char*, 8> kSectionObjectNames = {
            "libraryHierarchyItemModel",
            "projectsHierarchyItemModel",
            "bookmarksHierarchyItemModel",
            "tagsHierarchyItemModel",
            "resourcesHierarchyItemModel",
            "progressHierarchyItemModel",
            "eventHierarchyItemModel",
            "presetHierarchyItemModel"
        };

        if (index < 0 || index >= static_cast<int>(kSectionObjectNames.size()))
        {
            return QStringLiteral("unknownHierarchyItemModel");
        }

        return QString::fromLatin1(kSectionObjectNames.at(static_cast<std::size_t>(index)));
    }

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
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.store"),
            QStringLiteral("createDefaultSections"),
            QStringLiteral("count=%1").arg(static_cast<int>(sections.size())));
        return sections;
    }

    std::vector<std::unique_ptr<HierarchyItemListModel>> createSectionItemModels(
        const std::vector<std::unique_ptr<HierarchySectionModel>>& sections,
        QObject* parent)
    {
        std::vector<std::unique_ptr<HierarchyItemListModel>> sectionItemModels;
        sectionItemModels.reserve(sections.size());

        int sectionIndex = 0;
        for (const std::unique_ptr<HierarchySectionModel>& section : sections)
        {
            std::unique_ptr<HierarchyItemListModel> model = std::make_unique<HierarchyItemListModel>(parent);
            model->setObjectName(sectionObjectNameForIndex(sectionIndex));
            const QVector<SidebarHierarchyItem> items = section->items();
            model->setItems(items);
            WhatSon::Debug::trace(
                QStringLiteral("sidebar.store"),
                QStringLiteral("createSectionItemModel"),
                QStringLiteral("index=%1 section=%2 object=%3 count=%4")
                .arg(sectionIndex)
                .arg(section->sectionName())
                .arg(model->objectName())
                .arg(items.size()));
            sectionItemModels.push_back(std::move(model));
            ++sectionIndex;
        }

        return sectionItemModels;
    }
} // namespace

SidebarHierarchyStore::SidebarHierarchyStore(QObject* parent)
    : QObject(parent)
      , m_sections(createDefaultSections())
      , m_sectionItemModels(createSectionItemModels(m_sections, this))
{
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.store"),
        QStringLiteral("ctor"),
        QStringLiteral("sections=%1 sectionModels=%2 activeIndex=%3")
        .arg(static_cast<int>(m_sections.size()))
        .arg(static_cast<int>(m_sectionItemModels.size()))
        .arg(m_activeIndex));
}

SidebarHierarchyStore::~SidebarHierarchyStore() = default;

int SidebarHierarchyStore::activeIndex() const noexcept
{
    return m_activeIndex;
}

void SidebarHierarchyStore::setActiveIndex(int index)
{
    if (m_sectionItemModels.empty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.store"),
            QStringLiteral("setActiveIndexIgnored"),
            QStringLiteral("reason=noSectionModels requested=%1").arg(index));
        return;
    }

    const int maxIndex = static_cast<int>(m_sectionItemModels.size()) - 1;
    const int clampedIndex = std::clamp(index, 0, maxIndex);
    if (m_activeIndex == clampedIndex)
    {
        WhatSon::Debug::trace(
            QStringLiteral("sidebar.store"),
            QStringLiteral("setActiveIndexNoop"),
            QStringLiteral("requested=%1 clamped=%2").arg(index).arg(clampedIndex));
        return;
    }

    const int previousIndex = m_activeIndex;
    m_activeIndex = clampedIndex;
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.store"),
        QStringLiteral("setActiveIndex"),
        QStringLiteral("previous=%1 requested=%2 clamped=%3 section=%4")
        .arg(previousIndex)
        .arg(index)
        .arg(clampedIndex)
        .arg(m_sections.at(static_cast<std::size_t>(clampedIndex))->sectionName()));
    emit activeIndexChanged();
}

HierarchyItemListModel* SidebarHierarchyStore::itemModel() noexcept
{
    return itemModelForSection(m_activeIndex);
}

HierarchyItemListModel* SidebarHierarchyStore::itemModelForSection(int index) const noexcept
{
    if (m_sectionItemModels.empty())
    {
        return nullptr;
    }

    const int maxIndex = static_cast<int>(m_sectionItemModels.size()) - 1;
    const int safeIndex = std::clamp(index, 0, maxIndex);
    return m_sectionItemModels.at(static_cast<std::size_t>(safeIndex)).get();
}

QStringList SidebarHierarchyStore::sectionNames() const
{
    QStringList names;
    names.reserve(static_cast<qsizetype>(m_sections.size()));
    for (const std::unique_ptr<HierarchySectionModel>& section : m_sections)
    {
        names.push_back(section->sectionName());
    }
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.store"),
        QStringLiteral("sectionNames"),
        QStringLiteral("count=%1").arg(names.size()));
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
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.store"),
        QStringLiteral("toolbarIconNames"),
        QStringLiteral("count=%1").arg(iconNames.size()));
    return iconNames;
}

int SidebarHierarchyStore::sectionCount() const noexcept
{
    return static_cast<int>(m_sections.size());
}
