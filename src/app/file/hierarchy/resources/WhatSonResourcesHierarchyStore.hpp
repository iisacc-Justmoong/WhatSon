#pragma once

#include <QString>
#include <QStringList>

class WhatSonResourcesHierarchyStore
{
public:
    WhatSonResourcesHierarchyStore();
    ~WhatSonResourcesHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList resourcePaths() const;
    void setResourcePaths(QStringList values);

private:
    QString m_hubPath;
    QStringList m_resourcePaths;
};
