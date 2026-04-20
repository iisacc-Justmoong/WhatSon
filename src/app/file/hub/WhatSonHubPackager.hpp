#pragma once

#include <QString>

class WhatSonHubPackager
{
public:
    WhatSonHubPackager() = default;
    ~WhatSonHubPackager() = default;

    QString packageExtension() const;
    QString normalizePackagePath(const QString& hubPackagePath) const;
    bool createPackageRoot(
        const QString& hubPackagePath,
        QString* outPackagePath,
        QString* errorMessage) const;

private:
    bool ensureDirectory(const QString& absolutePath, QString* errorMessage) const;
    bool applyPackagePresentation(const QString& absolutePackagePath, QString* errorMessage) const;
};
