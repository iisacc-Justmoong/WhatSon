#pragma once

#include <QString>
#include <QStringList>
#include <QSet>

class WhatSonSystemIoGateway final
{
public:
    WhatSonSystemIoGateway();
    ~WhatSonSystemIoGateway();

    bool ensureDirectory(const QString& directoryPath, QString* errorMessage = nullptr) const;
    bool writeUtf8File(const QString& filePath, const QString& text, QString* errorMessage = nullptr) const;
    bool appendUtf8File(const QString& filePath, const QString& text, QString* errorMessage = nullptr) const;
    bool readUtf8File(const QString& filePath, QString* outText, QString* errorMessage = nullptr) const;
    bool removeFile(const QString& filePath, QString* errorMessage = nullptr) const;
    bool removeDirectoryRecursively(const QString& directoryPath, QString* errorMessage = nullptr) const;

    QStringList listFileNames(const QString& directoryPath) const;
    QStringList listDirectoryNames(const QString& directoryPath) const;

    bool exists(const QString& path) const;

private:
    QString normalizePath(const QString& path) const;
    void cacheEnsuredDirectory(const QString& normalizedDirectoryPath) const;
    void invalidateDirectoryCachePrefix(const QString& normalizedDirectoryPath) const;

    mutable QSet<QString> m_ensuredDirectories;
};
