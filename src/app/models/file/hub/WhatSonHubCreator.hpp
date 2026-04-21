#pragma once

#include "app/models/file/hub/WhatSonHubPackager.hpp"

#include <QString>
#include <QStringList>

class WhatSonHubCreator
{
public:
    explicit WhatSonHubCreator(
        QString workspaceRootPath,
        QString hubsRootPath = QStringLiteral("hubs"));
    ~WhatSonHubCreator();

    WhatSonHubCreator(const WhatSonHubCreator&) = delete;
    WhatSonHubCreator& operator=(const WhatSonHubCreator&) = delete;
    WhatSonHubCreator(WhatSonHubCreator&&) = default;
    WhatSonHubCreator& operator=(WhatSonHubCreator&&) = default;

    void setWorkspaceRootPath(QString workspaceRootPath);
    const QString& workspaceRootPath() const noexcept;

    void setHubsRootPath(QString hubsRootPath);
    const QString& hubsRootPath() const noexcept;

    QString creatorName() const;
    QStringList requiredRelativePaths(const QString& hubName) const;
    bool createHub(
        const QString& hubName,
        QString* outPackagePath,
        QString* errorMessage) const;
    bool createHubAtPath(
        const QString& hubPackagePath,
        QString* outPackagePath,
        QString* errorMessage) const;

    QString packageExtension() const;
    QString manifestFileName() const;
    QString hubContentsDirectoryName(const QString& hubName) const;
    QString hubResourcesDirectoryName(const QString& hubName) const;
    QString hubStatFileName(const QString& hubName) const;

protected:
    QString sanitizeHubName(const QString& hubName) const;
    QString joinPath(const QString& left, const QString& right) const;
    bool ensureDirectory(const QString& absolutePath, QString* errorMessage) const;
    bool writeTextFile(const QString& absolutePath, const QString& content, QString* errorMessage) const;

private:
    QString hubDirectoryPath(const QString& hubName) const;
    bool createHubScaffold(const QString& hubRootPath, const QString& hubName, QString* errorMessage) const;

    QString m_workspaceRootPath;
    QString m_hubsRootPath;
    WhatSonHubPackager m_packager;
};
