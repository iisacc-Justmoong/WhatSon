#pragma once

#include <QString>

struct ProgressHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

inline QString progressHierarchyIconName(const ProgressHierarchyItem& item)
{
    QString normalizedLabel = item.label.trimmed().toLower();
    normalizedLabel.remove(QLatin1Char(' '));
    normalizedLabel.remove(QLatin1Char('-'));
    normalizedLabel.remove(QLatin1Char('_'));
    if (normalizedLabel == QStringLiteral("ready") || normalizedLabel == QStringLiteral("firstdraft"))
    {
        return QStringLiteral("inlineinlineEdit");
    }
    if (normalizedLabel == QStringLiteral("modifieddraft"))
    {
        return QStringLiteral("rendererKit");
    }
    if (normalizedLabel == QStringLiteral("inprogress"))
    {
        return QStringLiteral("progressresume");
    }
    if (normalizedLabel == QStringLiteral("pending"))
    {
        return QStringLiteral("pending");
    }
    if (normalizedLabel == QStringLiteral("reviewing"))
    {
        return QStringLiteral("showLogs");
    }
    if (normalizedLabel == QStringLiteral("waitingforapproval"))
    {
        return QStringLiteral("toolWindowTimer");
    }
    if (normalizedLabel == QStringLiteral("done"))
    {
        return QStringLiteral("validator");
    }
    if (normalizedLabel == QStringLiteral("lagacy") || normalizedLabel == QStringLiteral("legacy"))
    {
        return QStringLiteral("nodesexcludeRoot");
    }
    if (normalizedLabel == QStringLiteral("archived"))
    {
        return QStringLiteral("projectModels");
    }
    if (normalizedLabel == QStringLiteral("deletereview"))
    {
        return QStringLiteral("generaldelete");
    }
    return QString();
}
