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
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    QStringList m_eventNames;
};
