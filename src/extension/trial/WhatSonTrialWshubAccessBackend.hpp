#pragma once

#include "app/register/WhatSonRegisterManager.hpp"
#include "extension/trial/WhatSonTrialActivationPolicy.hpp"
#include "extension/trial/WhatSonTrialClientIdentityStore.hpp"
#include "extension/trial/WhatSonTrialInstallStore.hpp"
#include "extension/trial/WhatSonTrialRegisterXml.hpp"

#include <QDate>
#include <QPointer>
#include <QString>

struct WhatSonTrialWshubAccessDecision final
{
    QString targetPath;
    QString normalizedTargetPath;
    QString denialReason;
    WhatSonTrialActivationState trialState;
    WhatSonTrialClientIdentity clientIdentity;
    WhatSonTrialClientIdentity hubIdentity;
    bool wshubTarget = false;
    bool allowed = true;
    bool restrictedByExpiredTrial = false;
    bool registerFilePresent = false;
    bool registerIntegrityVerified = false;
    bool clientKeyMatched = true;
    bool restrictedByMissingRegister = false;
    bool restrictedByClientKeyMismatch = false;
    bool restrictedByRegisterIntegrityFailure = false;

    bool operator==(const WhatSonTrialWshubAccessDecision& other) const = default;
};

class WhatSonTrialWshubAccessBackend final
{
public:
    explicit WhatSonTrialWshubAccessBackend(
        WhatSonTrialInstallStore installStore = WhatSonTrialInstallStore(),
        WhatSonRegisterManager* registerManager = nullptr,
        WhatSonTrialClientIdentityStore clientIdentityStore = WhatSonTrialClientIdentityStore(),
        WhatSonTrialRegisterXml registerXml = WhatSonTrialRegisterXml());

    WhatSonTrialWshubAccessDecision evaluateAccess(
        const QString& targetPath,
        const QDate& today = QDate::currentDate()) const;
    bool canAccess(
        const QString& targetPath,
        QString* denialReason = nullptr,
        const QDate& today = QDate::currentDate()) const;

private:
    static QString normalizeTargetPath(const QString& targetPath);
    static bool isWshubTargetPath(const QString& normalizedTargetPath);
    static bool isLocalComparableWshubTarget(const QString& normalizedTargetPath);

    WhatSonTrialInstallStore m_installStore;
    QPointer<WhatSonRegisterManager> m_registerManager;
    WhatSonTrialClientIdentityStore m_clientIdentityStore;
    WhatSonTrialRegisterXml m_registerXml;
};
