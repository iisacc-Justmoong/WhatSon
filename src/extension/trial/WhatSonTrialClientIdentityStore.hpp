#pragma once

#include "WhatSonTrialSecureStore.hpp"

#include <QString>

struct WhatSonTrialClientIdentity final
{
    QString deviceUuid;
    QString key;

    bool operator==(const WhatSonTrialClientIdentity& other) const = default;
};

class WhatSonTrialClientIdentityStore final
{
public:
    explicit WhatSonTrialClientIdentityStore(
        QString deviceUuidSettingsKey = defaultDeviceUuidSettingsKey(),
        QString clientKeySettingsKey = defaultClientKeySettingsKey(),
        QString registerIntegritySecretSettingsKey = defaultRegisterIntegritySecretSettingsKey(),
        WhatSonTrialSecureStore secureStore = WhatSonTrialSecureStore());

    static QString defaultDeviceUuidSettingsKey();
    static QString defaultClientKeySettingsKey();
    static QString defaultRegisterIntegritySecretSettingsKey();

    static QString normalizeDeviceUuid(QString value);
    static QString normalizeClientKey(QString value);
    static QString normalizeRegisterIntegritySecret(QString value);
    static WhatSonTrialClientIdentity normalizeIdentity(WhatSonTrialClientIdentity identity);

    QString deviceUuidSettingsKey() const;
    QString clientKeySettingsKey() const;
    QString registerIntegritySecretSettingsKey() const;

    QString loadDeviceUuid() const;
    QString loadClientKey() const;
    QString loadRegisterIntegritySecret() const;
    WhatSonTrialClientIdentity loadIdentity() const;
    WhatSonTrialClientIdentity ensureIdentity() const;
    QString ensureRegisterIntegritySecret() const;
    void storeIdentity(const WhatSonTrialClientIdentity& identity) const;
    void clear() const;

private:
    static QString createDeviceUuid();
    static QString createClientKey();
    static QString createRegisterIntegritySecret();

    void storeRegisterIntegritySecret(const QString& value) const;

    QString m_deviceUuidSettingsKey;
    QString m_clientKeySettingsKey;
    QString m_registerIntegritySecretSettingsKey;
    WhatSonTrialSecureStore m_secureStore;
};
