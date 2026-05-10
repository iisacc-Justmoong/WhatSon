#pragma once

#include <QString>
#include <QStringList>

class WhatSonLibraryHierarchyStore
{
public:
    WhatSonLibraryHierarchyStore();
    ~WhatSonLibraryHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList noteIds() const;
    void setNoteIds(QStringList values);

private:
    QString m_hubPath;
    QStringList m_noteIds;
};
