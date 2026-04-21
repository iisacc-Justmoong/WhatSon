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
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    QStringList m_resourcePaths;
};
