#pragma once

#include <QString>

struct BookmarksHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    QString iconSource;
    bool showChevron = true;
};

inline QString bookmarksHierarchyIconName(const BookmarksHierarchyItem&)
{
    return QStringLiteral("bookmarksbookmark");
}
