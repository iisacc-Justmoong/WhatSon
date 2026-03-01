#pragma once

#include <QString>
#include <QStringList>

class WhatSonEventHierarchyStore
{
public:
    WhatSonEventHierarchyStore();
    ~WhatSonEventHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList eventNames() const;
    void setEventNames(QStringList values);

private:
    QString m_hubPath;
    QStringList m_eventNames;
};
