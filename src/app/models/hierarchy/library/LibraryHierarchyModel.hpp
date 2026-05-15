#pragma once

#include <QString>

struct LibraryHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    QString folderPath;
    QString folderUuid;

    enum class SystemBucket
    {
        None,
        All,
        Draft,
        Today
    };

    SystemBucket systemBucket = SystemBucket::None;
    bool showChevron = true;
};

inline QString libraryHierarchyIconName(const LibraryHierarchyItem& item)
{
    switch (item.systemBucket)
    {
    case LibraryHierarchyItem::SystemBucket::All:
        return QStringLiteral("database");
    case LibraryHierarchyItem::SystemBucket::Draft:
        return QStringLiteral("generaledit");
    case LibraryHierarchyItem::SystemBucket::Today:
        return QStringLiteral("generalhistory");
    case LibraryHierarchyItem::SystemBucket::None:
        break;
    }

    if (item.accent && item.depth == 0)
    {
        return QStringLiteral("controllerFolder");
    }

    return QStringLiteral("objectGroup");
}
