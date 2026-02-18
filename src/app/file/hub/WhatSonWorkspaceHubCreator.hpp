#pragma once

#include "WhatSonHubCreator.hpp"

class WhatSonWorkspaceHubCreator : public WhatSonHubCreator
{
public:
    explicit WhatSonWorkspaceHubCreator(
        QString workspaceRootPath,
        QString hubsRootPath = QStringLiteral("hubs"));
    ~WhatSonWorkspaceHubCreator() override;

    void setHubsRootPath(QString hubsRootPath);
    const QString& hubsRootPath() const noexcept;

    QString creatorName() const override;
    QStringList requiredRelativePaths() const override;
    bool createHub(
        const QString& hubName,
        QString* outPackagePath,
        QString* errorMessage) const override;

    QString packageExtension() const;
    QString manifestFileName() const;

private:
    QString hubDirectoryPath(const QString& hubName) const;
    bool createHubScaffold(const QString& hubRootPath, QString* errorMessage) const;
    bool packageHubDirectory(
        const QString& hubRootPath,
        const QString& packagePath,
        QString* errorMessage) const;

    QString m_hubsRootPath;
};
