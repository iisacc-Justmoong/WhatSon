#pragma once

#include <QString>

struct TagsHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

inline QString tagsHierarchyIconName(const TagsHierarchyItem&)
{
    return QStringLiteral("customFolder");
}
