#include "extension/trial/WhatSonTrialWshubAccessBackend.hpp"

#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include <utility>

namespace
{
    QString buildExpiredTrialMessage(const WhatSonTrialActivationState& state)
    {
        const QString expirationDateText = state.lastActiveDate.isValid()
                                               ? state.lastActiveDate.toString(Qt::ISODate)
                                               : QStringLiteral("unknown");
        return QStringLiteral(
            "WhatSon trial expired after the 90-day evaluation window. "
            ".wshub access is disabled after %1.")
            .arg(expirationDateText);
    }

    QString buildKeyMismatchMessage()
    {
        return QStringLiteral(
            "WhatSon trial hub access is denied because the in-app client key "
            "does not match .whatson/trial_register.xml.");
    }

    QString buildMissingRegisterMessage()
    {
        return QStringLiteral(
            "WhatSon trial hub access is denied because .whatson/trial_register.xml "
            "is missing.");
    }

    QString buildInvalidRegisterMessage()
    {
        return QStringLiteral(
            "WhatSon trial hub access is denied because .whatson/trial_register.xml "
            "is missing a valid client key.");
    }

    QString buildInvalidRegisterIntegrityMessage()
    {
        return QStringLiteral(
            "WhatSon trial hub access is denied because .whatson/trial_register.xml "
            "failed integrity verification.");
    }
}

WhatSonTrialWshubAccessBackend::WhatSonTrialWshubAccessBackend(
    WhatSonTrialInstallStore installStore,
    WhatSonRegisterManager* registerManager,
    WhatSonTrialClientIdentityStore clientIdentityStore,
    WhatSonTrialRegisterXml registerXml)
    : m_installStore(std::move(installStore))
    , m_registerManager(registerManager)
    , m_clientIdentityStore(std::move(clientIdentityStore))
    , m_registerXml(std::move(registerXml))
{
}

WhatSonTrialWshubAccessDecision WhatSonTrialWshubAccessBackend::evaluateAccess(
    const QString& targetPath,
    const QDate& today) const
{
    WhatSonTrialWshubAccessDecision decision;
    decision.targetPath = targetPath;
    decision.normalizedTargetPath = normalizeTargetPath(targetPath);
    decision.wshubTarget = isWshubTargetPath(decision.normalizedTargetPath);

    if (!decision.wshubTarget)
    {
        return decision;
    }

    WhatSonTrialActivationPolicy policy(m_installStore);
    policy.setRegisterManager(m_registerManager);
    decision.trialState = policy.refreshForDate(today);
    decision.allowed = decision.trialState.active;
    decision.restrictedByExpiredTrial = !decision.allowed;
    if (!decision.allowed)
    {
        decision.denialReason = buildExpiredTrialMessage(decision.trialState);
        return decision;
    }

    if (decision.trialState.bypassedByAuthentication)
    {
        return decision;
    }

    if (!isLocalComparableWshubTarget(decision.normalizedTargetPath))
    {
        return decision;
    }

    decision.registerFilePresent = m_registerXml.exists(decision.normalizedTargetPath);
    if (!decision.registerFilePresent)
    {
        decision.allowed = false;
        decision.clientKeyMatched = false;
        decision.restrictedByMissingRegister = true;
        decision.denialReason = buildMissingRegisterMessage();
        return decision;
    }

    QString registerError;
    decision.clientIdentity = m_clientIdentityStore.ensureIdentity();
    const WhatSonTrialRegisterLoadResult registerLoad = m_registerXml.loadRegister(decision.normalizedTargetPath);
    decision.hubIdentity = registerLoad.identity;
    decision.registerIntegrityVerified = registerLoad.integrityVerified;
    registerError = registerLoad.errorMessage;
    if (decision.hubIdentity.key.isEmpty())
    {
        decision.allowed = false;
        decision.clientKeyMatched = false;
        decision.restrictedByRegisterIntegrityFailure =
            registerError.contains(QStringLiteral("integrity"), Qt::CaseInsensitive)
            || registerError.contains(QStringLiteral("signature"), Qt::CaseInsensitive);
        decision.restrictedByClientKeyMismatch = !decision.restrictedByRegisterIntegrityFailure;
        decision.denialReason = registerError.trimmed().isEmpty()
                                    ? (decision.restrictedByRegisterIntegrityFailure
                                           ? buildInvalidRegisterIntegrityMessage()
                                           : buildInvalidRegisterMessage())
                                    : registerError.trimmed();
        return decision;
    }

    decision.clientKeyMatched = decision.clientIdentity.key == decision.hubIdentity.key;
    if (!decision.clientKeyMatched)
    {
        decision.allowed = false;
        decision.restrictedByClientKeyMismatch = true;
        decision.denialReason = buildKeyMismatchMessage();
    }

    return decision;
}

bool WhatSonTrialWshubAccessBackend::canAccess(
    const QString& targetPath,
    QString* denialReason,
    const QDate& today) const
{
    const WhatSonTrialWshubAccessDecision decision = evaluateAccess(targetPath, today);
    if (denialReason != nullptr)
    {
        *denialReason = decision.denialReason;
    }
    return decision.allowed;
}

QString WhatSonTrialWshubAccessBackend::normalizeTargetPath(const QString& targetPath)
{
    const QString trimmedTargetPath = targetPath.trimmed();
    if (trimmedTargetPath.isEmpty())
    {
        return {};
    }

    const QUrl targetUrl(trimmedTargetPath);
    if (targetUrl.isValid())
    {
        if (targetUrl.isLocalFile())
        {
            return QDir::cleanPath(targetUrl.toLocalFile());
        }

        if (!targetUrl.scheme().isEmpty())
        {
            return trimmedTargetPath;
        }
    }

    return QDir::cleanPath(trimmedTargetPath);
}

bool WhatSonTrialWshubAccessBackend::isWshubTargetPath(const QString& normalizedTargetPath)
{
    if (normalizedTargetPath.isEmpty())
    {
        return false;
    }

    const QUrl targetUrl(normalizedTargetPath);
    if (targetUrl.isValid())
    {
        if (targetUrl.isLocalFile())
        {
            const QFileInfo fileInfo(targetUrl.toLocalFile());
            return fileInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive);
        }

        if (!targetUrl.scheme().isEmpty())
        {
            return targetUrl.path().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive)
                   || normalizedTargetPath.endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive);
        }
    }

    const QFileInfo fileInfo(normalizedTargetPath);
    return fileInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive);
}

bool WhatSonTrialWshubAccessBackend::isLocalComparableWshubTarget(const QString& normalizedTargetPath)
{
    if (normalizedTargetPath.isEmpty())
    {
        return false;
    }

    const QUrl targetUrl(normalizedTargetPath);
    if (targetUrl.isValid() && !targetUrl.scheme().isEmpty())
    {
        return targetUrl.isLocalFile();
    }

    return true;
}
