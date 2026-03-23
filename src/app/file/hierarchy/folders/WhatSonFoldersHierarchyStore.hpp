#pragma once

#include "file/hierarchy/WhatSonFolderDepthEntry.hpp"

#include <QString>
#include <QVector>

class WhatSonFoldersHierarchyStore
{
public:
    WhatSonFoldersHierarchyStore();
    ~WhatSonFoldersHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QVector<WhatSonFolderDepthEntry> folderEntries() const;
    void setFolderEntries(QVector<WhatSonFolderDepthEntry> entries);
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    QVector<WhatSonFolderDepthEntry> m_folderEntries;
};
