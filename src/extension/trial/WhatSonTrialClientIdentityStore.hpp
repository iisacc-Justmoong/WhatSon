#pragma once

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
        QString clientKeySettingsKey = defaultClientKeySettingsKey());

    static QString defaultDeviceUuidSettingsKey();
    static QString defaultClientKeySettingsKey();

    static QString normalizeDeviceUuid(QString value);
    static QString normalizeClientKey(QString value);
    static WhatSonTrialClientIdentity normalizeIdentity(WhatSonTrialClientIdentity identity);

    QString deviceUuidSettingsKey() const;
    QString clientKeySettingsKey() const;

    QString loadDeviceUuid() const;
    QString loadClientKey() const;
    WhatSonTrialClientIdentity loadIdentity() const;
    WhatSonTrialClientIdentity ensureIdentity() const;
    void storeIdentity(const WhatSonTrialClientIdentity& identity) const;
    void clear() const;

private:
    static QString createDeviceUuid();
    static QString createClientKey();

    QString m_deviceUuidSettingsKey;
    QString m_clientKeySettingsKey;
};
