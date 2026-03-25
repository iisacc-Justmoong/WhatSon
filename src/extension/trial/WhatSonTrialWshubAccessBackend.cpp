#include "WhatSonTrialWshubAccessBackend.hpp"

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
}

WhatSonTrialWshubAccessBackend::WhatSonTrialWshubAccessBackend(WhatSonTrialInstallStore installStore)
    : m_installStore(std::move(installStore))
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
    decision.trialState = policy.refreshForDate(today);
    decision.allowed = decision.trialState.active;
    decision.restrictedByExpiredTrial = !decision.allowed;
    if (!decision.allowed)
    {
        decision.denialReason = buildExpiredTrialMessage(decision.trialState);
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
