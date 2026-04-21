#pragma once

#include <QString>
#include <QStringList>

class WhatSonPresetHierarchyStore
{
public:
    WhatSonPresetHierarchyStore();
    ~WhatSonPresetHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QStringList presetNames() const;
    void setPresetNames(QStringList values);
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    QStringList m_presetNames;
};
