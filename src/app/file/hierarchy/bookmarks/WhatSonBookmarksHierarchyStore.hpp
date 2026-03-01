#pragma once

#include <QString>
#include <QStringList>

class WhatSonBookmarksHierarchyStore
{
public:
    WhatSonBookmarksHierarchyStore();
    ~WhatSonBookmarksHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList bookmarkIds() const;
    void setBookmarkIds(QStringList values);

private:
    QString m_hubPath;
    QStringList m_bookmarkIds;
};
