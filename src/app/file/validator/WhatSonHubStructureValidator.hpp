#pragma once

#include <QString>
#include <QStringList>

class WhatSonHubStructureValidator final
{
public:
    WhatSonHubStructureValidator();
    ~WhatSonHubStructureValidator();

    bool resolveContentsDirectories(
        const QString& wshubPath,
        QStringList* outContentsDirectories,
        QString* errorMessage = nullptr) const;
    QStringList resolveLibraryRoots(const QString& wshubPath) const;
    QString resolvePrimaryLibraryPath(const QString& wshubPath, QString* errorMessage = nullptr) const;
    QString resolveHubStatPath(const QString& wshubPath) const;

private:
    QString normalizePath(const QString& path) const;
};
