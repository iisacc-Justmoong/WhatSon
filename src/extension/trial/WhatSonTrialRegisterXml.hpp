#pragma once

#include "WhatSonTrialClientIdentityStore.hpp"

#include <QString>

class WhatSonTrialRegisterXml final
{
public:
    static QString registerFileName();

    QString registerFilePath(const QString& hubRootPath) const;
    bool exists(const QString& hubRootPath) const;
    WhatSonTrialClientIdentity loadRegister(const QString& hubRootPath, QString* errorMessage = nullptr) const;
    bool writeRegister(
        const QString& hubRootPath,
        const WhatSonTrialClientIdentity& identity,
        QString* errorMessage = nullptr) const;

private:
    static QString normalizeHubRootPath(const QString& hubRootPath);
};
