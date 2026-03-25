#include "WhatSonTrialActivationPolicy.hpp"

#include <QtGlobal>

#include <utility>

WhatSonTrialActivationPolicy::WhatSonTrialActivationPolicy(QObject* parent)
    : QObject(parent)
{
}

WhatSonTrialActivationPolicy::WhatSonTrialActivationPolicy(WhatSonTrialInstallStore installStore, QObject* parent)
    : QObject(parent)
    , m_installStore(std::move(installStore))
{
}

QDate WhatSonTrialActivationPolicy::installDate() const noexcept
{
    return m_state.installDate;
}

QDate WhatSonTrialActivationPolicy::lastActiveDate() const noexcept
{
    return m_state.lastActiveDate;
}

int WhatSonTrialActivationPolicy::trialLengthDays() const noexcept
{
    return kDefaultTrialLengthDays;
}

int WhatSonTrialActivationPolicy::elapsedDays() const noexcept
{
    return m_state.elapsedDays;
}

int WhatSonTrialActivationPolicy::remainingDays() const noexcept
{
    return m_state.remainingDays;
}

bool WhatSonTrialActivationPolicy::active() const noexcept
{
    return m_state.active;
}

bool WhatSonTrialActivationPolicy::bypassedByAuthentication() const noexcept
{
    return m_state.bypassedByAuthentication;
}

void WhatSonTrialActivationPolicy::setRegisterManager(WhatSonRegisterManager* registerManager)
{
    m_registerManager = registerManager;
}

WhatSonTrialActivationState WhatSonTrialActivationPolicy::currentState() const noexcept
{
    return m_state;
}

WhatSonTrialActivationState WhatSonTrialActivationPolicy::refreshForDate(const QDate& today)
{
    const QDate evaluationDate = today.isValid() ? today : QDate::currentDate();
    const QDate installDate = m_installStore.ensureInstallDate(evaluationDate);
    const WhatSonTrialActivationState nextState =
        m_registerManager != nullptr && m_registerManager->authenticated()
            ? buildAuthenticatedState(installDate)
            : buildState(installDate, evaluationDate);
    setState(nextState);
    return m_state;
}

void WhatSonTrialActivationPolicy::refresh()
{
    refreshForDate(QDate::currentDate());
}

WhatSonTrialActivationState WhatSonTrialActivationPolicy::buildState(const QDate& installDate, const QDate& today)
{
    WhatSonTrialActivationState state;
    state.trialLengthDays = kDefaultTrialLengthDays;

    if (!installDate.isValid())
    {
        return state;
    }

    state.installDate = installDate;
    state.lastActiveDate = installDate.addDays(kDefaultTrialLengthDays - 1);
    state.elapsedDays = qMax(0, installDate.daysTo(today));
    state.active = state.elapsedDays < kDefaultTrialLengthDays;
    state.remainingDays = state.active ? kDefaultTrialLengthDays - state.elapsedDays : 0;
    return state;
}

WhatSonTrialActivationState WhatSonTrialActivationPolicy::buildAuthenticatedState(const QDate& installDate)
{
    WhatSonTrialActivationState state;
    state.trialLengthDays = kDefaultTrialLengthDays;
    state.installDate = installDate;
    state.active = true;
    state.remainingDays = kDefaultTrialLengthDays;
    state.bypassedByAuthentication = true;
    return state;
}

void WhatSonTrialActivationPolicy::setState(const WhatSonTrialActivationState& state)
{
    if (m_state == state)
    {
        return;
    }

    m_state = state;
    emit stateChanged();
}
