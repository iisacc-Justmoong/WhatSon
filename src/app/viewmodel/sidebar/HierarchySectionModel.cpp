#include "HierarchySectionModel.hpp"

#include <initializer_list>

namespace
{
    QVector<SidebarHierarchyItem> makeItems(std::initializer_list<SidebarHierarchyItem> items)
    {
        return QVector<SidebarHierarchyItem>(items);
    }
} // namespace

HierarchySectionModel::~HierarchySectionModel() = default;

QString LibraryModel::sectionName() const
{
    return QStringLiteral("Library");
}

QString LibraryModel::toolbarIconName() const
{
    return QStringLiteral("libraryFolder");
}

QVector<SidebarHierarchyItem> LibraryModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Library"), false, true},
        {1, false, QStringLiteral("Collections"), false, true},
        {2, true, QStringLiteral("Brand Kit"), false, true},
        {1, false, QStringLiteral("Templates"), false, true},
        {1, false, QStringLiteral("Assets"), false, true},
        {1, false, QStringLiteral("References"), false, true},
        {0, false, QStringLiteral("Archive"), false, true},
        {0, false, QStringLiteral("Inbox"), false, true},
        {0, false, QStringLiteral("Today"), false, true},
        {0, false, QStringLiteral("Drafts"), false, true}
    });
}

QString ProjectsModel::sectionName() const
{
    return QStringLiteral("Projects");
}

QString ProjectsModel::toolbarIconName() const
{
    return QStringLiteral("projectStructure");
}

QVector<SidebarHierarchyItem> ProjectsModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Projects"), false, true},
        {1, false, QStringLiteral("Active"), false, true},
        {2, true, QStringLiteral("Alpha Launch"), false, true},
        {1, false, QStringLiteral("Client A"), false, true},
        {1, false, QStringLiteral("Client B"), false, true},
        {1, false, QStringLiteral("Internal"), false, true},
        {0, false, QStringLiteral("Roadmap"), false, true},
        {0, false, QStringLiteral("Backlog"), false, true},
        {0, false, QStringLiteral("Milestones"), false, true},
        {0, false, QStringLiteral("Completed"), false, true}
    });
}

QString BookmarksModel::sectionName() const
{
    return QStringLiteral("Bookmarks");
}

QString BookmarksModel::toolbarIconName() const
{
    return QStringLiteral("bookmarksList");
}

QVector<SidebarHierarchyItem> BookmarksModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Saved"), false, true},
        {1, false, QStringLiteral("Recent"), false, true},
        {2, true, QStringLiteral("Weekly Brief"), false, true},
        {1, false, QStringLiteral("Research"), false, true},
        {1, false, QStringLiteral("Inspiration"), false, true},
        {1, false, QStringLiteral("Snippets"), false, true},
        {0, false, QStringLiteral("Pinboard"), false, true},
        {0, false, QStringLiteral("Watchlist"), false, true},
        {0, false, QStringLiteral("Shared"), false, true},
        {0, false, QStringLiteral("Archived"), false, true}
    });
}

QString TagsModel::sectionName() const
{
    return QStringLiteral("Tags");
}

QString TagsModel::toolbarIconName() const
{
    return QStringLiteral("currentBranch");
}

QVector<SidebarHierarchyItem> TagsModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Tags"), false, true},
        {1, false, QStringLiteral("Product"), false, true},
        {2, true, QStringLiteral("Critical"), false, true},
        {1, false, QStringLiteral("Design"), false, true},
        {1, false, QStringLiteral("Marketing"), false, true},
        {1, false, QStringLiteral("Operations"), false, true},
        {0, false, QStringLiteral("Team"), false, true},
        {0, false, QStringLiteral("Personal"), false, true},
        {0, false, QStringLiteral("Ideas"), false, true},
        {0, false, QStringLiteral("Later"), false, true}
    });
}

QString ResourcesModel::sectionName() const
{
    return QStringLiteral("Resources");
}

QString ResourcesModel::toolbarIconName() const
{
    return QStringLiteral("imageToImage");
}

QVector<SidebarHierarchyItem> ResourcesModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Resources"), false, true},
        {1, false, QStringLiteral("Documents"), false, true},
        {2, true, QStringLiteral("Brand Guide"), false, true},
        {1, false, QStringLiteral("Playbooks"), false, true},
        {1, false, QStringLiteral("Contracts"), false, true},
        {1, false, QStringLiteral("Datasets"), false, true},
        {0, false, QStringLiteral("Libraries"), false, true},
        {0, false, QStringLiteral("Templates"), false, true},
        {0, false, QStringLiteral("Integrations"), false, true},
        {0, false, QStringLiteral("Attachments"), false, true}
    });
}

QString ProgressModel::sectionName() const
{
    return QStringLiteral("Progress");
}

QString ProgressModel::toolbarIconName() const
{
    return QStringLiteral("chartBar");
}

QVector<SidebarHierarchyItem> ProgressModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Progress"), false, true},
        {1, false, QStringLiteral("This Week"), false, true},
        {2, true, QStringLiteral("Launch Readiness"), false, true},
        {1, false, QStringLiteral("Campaign A"), false, true},
        {1, false, QStringLiteral("Campaign B"), false, true},
        {1, false, QStringLiteral("Experiments"), false, true},
        {0, false, QStringLiteral("Goals"), false, true},
        {0, false, QStringLiteral("Metrics"), false, true},
        {0, false, QStringLiteral("Reviews"), false, true},
        {0, false, QStringLiteral("Done"), false, true}
    });
}

QString EventModel::sectionName() const
{
    return QStringLiteral("Event");
}

QString EventModel::toolbarIconName() const
{
    return QStringLiteral("dataView");
}

QVector<SidebarHierarchyItem> EventModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Events"), false, true},
        {1, false, QStringLiteral("Calendar"), false, true},
        {2, true, QStringLiteral("Product Sync"), false, true},
        {1, false, QStringLiteral("Client Review"), false, true},
        {1, false, QStringLiteral("Workshop"), false, true},
        {1, false, QStringLiteral("Webinar"), false, true},
        {0, false, QStringLiteral("Deadlines"), false, true},
        {0, false, QStringLiteral("Releases"), false, true},
        {0, false, QStringLiteral("Follow-ups"), false, true},
        {0, false, QStringLiteral("History"), false, true}
    });
}

QString PresetModel::sectionName() const
{
    return QStringLiteral("Preset");
}

QString PresetModel::toolbarIconName() const
{
    return QStringLiteral("dataFile");
}

QVector<SidebarHierarchyItem> PresetModel::items() const
{
    return makeItems({
        {0, false, QStringLiteral("Presets"), false, true},
        {1, false, QStringLiteral("Writing"), false, true},
        {2, true, QStringLiteral("Executive Summary"), false, true},
        {1, false, QStringLiteral("Blog Draft"), false, true},
        {1, false, QStringLiteral("Meeting Notes"), false, true},
        {1, false, QStringLiteral("Content Plan"), false, true},
        {0, false, QStringLiteral("Automation"), false, true},
        {0, false, QStringLiteral("Tag Set"), false, true},
        {0, false, QStringLiteral("Saved Filters"), false, true},
        {0, false, QStringLiteral("Quick Actions"), false, true}
    });
}
