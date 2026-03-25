#pragma once

#include "WhatSonTrialClientIdentityStore.hpp"

#include <QString>

struct WhatSonTrialRegisterLoadResult final
{
    WhatSonTrialClientIdentity identity;
    QString errorMessage;
    bool integrityVerified = false;
};

class WhatSonTrialRegisterXml final
{
public:
    explicit WhatSonTrialRegisterXml(
        WhatSonTrialClientIdentityStore clientIdentityStore = WhatSonTrialClientIdentityStore());

    static QString registerFileName();
    static QString signatureAlgorithmName();

    QString registerFilePath(const QString& hubRootPath) const;
    bool exists(const QString& hubRootPath) const;
    WhatSonTrialRegisterLoadResult loadRegister(const QString& hubRootPath) const;
    bool writeRegister(
        const QString& hubRootPath,
        const WhatSonTrialClientIdentity& identity,
        QString* errorMessage = nullptr) const;

private:
    static QString normalizeHubRootPath(const QString& hubRootPath);

    WhatSonTrialClientIdentityStore m_clientIdentityStore;
};
