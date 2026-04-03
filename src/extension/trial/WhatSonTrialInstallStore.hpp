#pragma once

#include "WhatSonTrialClientIdentityStore.hpp"
#include "WhatSonTrialSecureStore.hpp"

#include <QDate>
#include <QString>

class WhatSonTrialInstallStore final
{
public:
    explicit WhatSonTrialInstallStore(
        QString installDateSettingsKey = defaultInstallDateSettingsKey(),
        WhatSonTrialClientIdentityStore clientIdentityStore = WhatSonTrialClientIdentityStore(),
        WhatSonTrialSecureStore secureStore = WhatSonTrialSecureStore());

    static QString defaultInstallDateSettingsKey();

    QString installDateSettingsKey() const;
    QDate loadInstallDate() const;
    QDate ensureInstallDate(const QDate& today = QDate::currentDate()) const;
    void storeInstallDate(const QDate& installDate) const;
    void clear() const;

private:
    QString m_installDateSettingsKey;
    WhatSonTrialClientIdentityStore m_clientIdentityStore;
    WhatSonTrialSecureStore m_secureStore;
};
