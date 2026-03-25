#include "WhatSonRegisterManager.hpp"

#include <QSettings>

#include <utility>

WhatSonRegisterManager::WhatSonRegisterManager(QString authenticatedSettingsKey, QObject* parent)
    : QObject(parent)
    , m_authenticatedSettingsKey(std::move(authenticatedSettingsKey))
{
    reload();
}

QString WhatSonRegisterManager::defaultAuthenticatedSettingsKey()
{
    return QStringLiteral("register/authenticated");
}

QString WhatSonRegisterManager::authenticatedSettingsKey() const
{
    return m_authenticatedSettingsKey;
}

bool WhatSonRegisterManager::authenticated() const noexcept
{
    return m_authenticated;
}

void WhatSonRegisterManager::reload()
{
    QSettings settings;
    setAuthenticatedState(settings.value(m_authenticatedSettingsKey, false).toBool(), true);
}

void WhatSonRegisterManager::setAuthenticated(const bool authenticated)
{
    QSettings settings;
    settings.setValue(m_authenticatedSettingsKey, authenticated);
    settings.sync();
    setAuthenticatedState(authenticated, true);
}

void WhatSonRegisterManager::clearAuthentication()
{
    setAuthenticated(false);
}

void WhatSonRegisterManager::setAuthenticatedState(const bool authenticated, const bool emitSignal)
{
    if (m_authenticated == authenticated)
    {
        return;
    }

    m_authenticated = authenticated;
    if (emitSignal)
    {
        emit authenticatedChanged();
    }
}
