#pragma once

#include <QString>
#include <QStringList>

class WhatSonHubCreator
{
public:
    explicit WhatSonHubCreator(QString workspaceRootPath);
    virtual ~WhatSonHubCreator();

    WhatSonHubCreator(const WhatSonHubCreator&) = delete;
    WhatSonHubCreator& operator=(const WhatSonHubCreator&) = delete;
    WhatSonHubCreator(WhatSonHubCreator&&) = default;
    WhatSonHubCreator& operator=(WhatSonHubCreator&&) = default;

    void setWorkspaceRootPath(QString workspaceRootPath);
    const QString& workspaceRootPath() const noexcept;

    virtual QString creatorName() const = 0;
    virtual QStringList requiredRelativePaths() const = 0;
    virtual bool createHub(
        const QString& hubName,
        QString* outPackagePath,
        QString* errorMessage) const = 0;

protected:
    QString sanitizeHubName(const QString& hubName) const;
    QString joinPath(const QString& left, const QString& right) const;
    bool ensureDirectory(const QString& absolutePath, QString* errorMessage) const;
    bool writeTextFile(const QString& absolutePath, const QString& content, QString* errorMessage) const;

private:
    QString m_workspaceRootPath;
};
