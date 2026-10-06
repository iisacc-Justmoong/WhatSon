#pragma once

#include "app/models/hierarchy/tags/WhatSonTagDepthEntry.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonTagsHierarchyStore
{
public:
    WhatSonTagsHierarchyStore();
    ~WhatSonTagsHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList tagNames() const;
    void setTagNames(QStringList values);
    QVector<WhatSonTagDepthEntry> tagEntries() const;
    void setTagEntries(QVector<WhatSonTagDepthEntry> entries);
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    QStringList m_tagNames;
    QVector<WhatSonTagDepthEntry> m_tagEntries;
};
