#pragma once

#include <QString>
#include <QStringList>

class WhatSonProgressHierarchyStore
{
public:
    WhatSonProgressHierarchyStore();
    ~WhatSonProgressHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    int progressValue() const noexcept;
    void setProgressValue(int progressValue) noexcept;

    QStringList progressStates() const;
    void setProgressStates(QStringList progressStates);
    bool writeToFile(const QString& filePath, QString* errorMessage = nullptr) const;

private:
    QString m_hubPath;
    int m_progressValue = 0;
    QStringList m_progressStates;
};
