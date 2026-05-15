#pragma once

#include <QString>

struct ResourcesHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    int count = 0;
    bool showChevron = true;
    QString key;
    QString kind;
    QString bucket;
    QString type;
    QString format;
    QString resourceId;
    QString resourcePath;
    QString assetPath;
};

inline QString resourcesHierarchyIconName(const ResourcesHierarchyItem& item)
{
    if (item.kind == QStringLiteral("asset"))
    {
        if (item.type == QStringLiteral("image"))
        {
            return QStringLiteral("imageToImage");
        }
        if (item.type == QStringLiteral("video"))
        {
            return QStringLiteral("generalshow");
        }
        if (item.type == QStringLiteral("document"))
        {
            return QStringLiteral("dataView");
        }
        if (item.type == QStringLiteral("link"))
        {
            return QStringLiteral("cwmPermissionView");
        }
        if (item.type == QStringLiteral("audio"))
        {
            return QStringLiteral("audioToAudio");
        }
        if (item.type == QStringLiteral("archive"))
        {
            return QStringLiteral("sortByType");
        }
        if (item.type == QStringLiteral("model"))
        {
            return QStringLiteral("dataFile");
        }
    }
    return QStringLiteral("virtualFolder");
}
