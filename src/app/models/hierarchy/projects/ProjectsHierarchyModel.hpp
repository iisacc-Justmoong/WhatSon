#pragma once

#include <QString>

struct ProjectsHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

inline QString projectsHierarchyIconName(const ProjectsHierarchyItem&)
{
    return QStringLiteral("customFolder");
}
