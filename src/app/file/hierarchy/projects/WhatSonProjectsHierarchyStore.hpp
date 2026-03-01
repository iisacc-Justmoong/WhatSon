#pragma once

#include <QString>
#include <QStringList>

class WhatSonProjectsHierarchyStore
{
public:
    WhatSonProjectsHierarchyStore();
    ~WhatSonProjectsHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList projectNames() const;
    void setProjectNames(QStringList values);

private:
    QString m_hubPath;
    QStringList m_projectNames;
};
