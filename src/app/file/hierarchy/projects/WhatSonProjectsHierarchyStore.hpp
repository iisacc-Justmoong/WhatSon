#pragma once

#include "../WhatSonFolderDepthEntry.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

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
    QVector<WhatSonFolderDepthEntry> folderEntries() const;
    void setFolderEntries(QVector<WhatSonFolderDepthEntry> entries);
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    QStringList m_projectNames;
    QVector<WhatSonFolderDepthEntry> m_folderEntries;
};
