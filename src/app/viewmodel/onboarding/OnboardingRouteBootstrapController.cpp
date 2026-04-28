#include "app/viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/viewmodel/onboarding/IOnboardingHubController.hpp"

namespace
{
    constexpr auto kWorkspaceRoutePath = "/";
    constexpr auto kOnboardingRoutePath = "/onboarding";
}

OnboardingRouteBootstrapController::OnboardingRouteBootstrapController(QObject* parent)
    : QObject(parent)
{
}

bool OnboardingRouteBootstrapController::embeddedOnboardingEnabled() const noexcept
{
    return m_embeddedOnboardingEnabled;
}

bool OnboardingRouteBootstrapController::embeddedOnboardingVisible() const noexcept
{
    return m_embeddedOnboardingVisible;
}

bool OnboardingRouteBootstrapController::routeCommitPending() const noexcept
{
    return m_routeCommitPending;
}

QString OnboardingRouteBootstrapController::startupRoutePath() const
{
    return m_startupRoutePath;
}

void OnboardingRouteBootstrapController::setHubController(IOnboardingHubController* controller)
{
    if (!WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("OnboardingRouteBootstrapController::setHubController")))
    {
        return;
    }

    m_hubController = controller;
}

void OnboardingRouteBootstrapController::configure(const bool embeddedEnabled, const bool startupWorkspaceReady)
{
    setEmbeddedOnboardingEnabled(embeddedEnabled);
    setRouteCommitPending(false);
    setEmbeddedOnboardingVisible(embeddedEnabled && !startupWorkspaceReady);
    setStartupRoutePath(m_embeddedOnboardingVisible
                            ? QString::fromLatin1(kOnboardingRoutePath)
                            : QString::fromLatin1(kWorkspaceRoutePath));
}

void OnboardingRouteBootstrapController::handleHubLoaded()
{
    if (!m_embeddedOnboardingEnabled || m_routeCommitPending)
    {
        return;
    }

    if (m_hubController != nullptr)
    {
        m_hubController->beginWorkspaceTransition();
    }

    setRouteCommitPending(true);
    setEmbeddedOnboardingVisible(false);
    emit routeSyncRequested(
        QString::fromLatin1(kWorkspaceRoutePath),
        true,
        QStringLiteral("embeddedHubLoaded"));
}

void OnboardingRouteBootstrapController::handleOperationFailed(const QString& message)
{
    if (!m_routeCommitPending)
    {
        return;
    }

    if (m_hubController != nullptr)
    {
        m_hubController->failWorkspaceTransition(message);
    }

    setRouteCommitPending(false);
    setEmbeddedOnboardingVisible(true);
    emit routeSyncRequested(
        QString::fromLatin1(kOnboardingRoutePath),
        true,
        QStringLiteral("embeddedOperationFailed"));
}

void OnboardingRouteBootstrapController::handlePageStackNavigated(const QString& path)
{
    if (!m_routeCommitPending || path.trimmed() != QString::fromLatin1(kWorkspaceRoutePath))
    {
        return;
    }

    setRouteCommitPending(false);
    if (m_hubController != nullptr)
    {
        m_hubController->completeWorkspaceTransition();
    }
}

void OnboardingRouteBootstrapController::handlePageStackNavigationFailed(const QString& path)
{
    Q_UNUSED(path);

    if (!m_routeCommitPending)
    {
        return;
    }

    setRouteCommitPending(false);
    setEmbeddedOnboardingVisible(true);
    if (m_hubController != nullptr)
    {
        m_hubController->failWorkspaceTransition(
            QStringLiteral("Failed to open the WhatSon workspace after onboarding."));
    }

    emit routeSyncRequested(
        QString::fromLatin1(kOnboardingRoutePath),
        false,
        QStringLiteral("navigationFailed"));
}

void OnboardingRouteBootstrapController::reopenEmbeddedOnboarding()
{
    if (!m_embeddedOnboardingEnabled)
    {
        return;
    }

    setRouteCommitPending(false);
    setEmbeddedOnboardingVisible(true);
    emit routeSyncRequested(
        QString::fromLatin1(kOnboardingRoutePath),
        false,
        QStringLiteral("reopenEmbeddedOnboarding"));
}

void OnboardingRouteBootstrapController::dismissEmbeddedOnboarding()
{
    if (!m_embeddedOnboardingEnabled)
    {
        return;
    }

    setRouteCommitPending(false);
    setEmbeddedOnboardingVisible(false);
    emit routeSyncRequested(
        QString::fromLatin1(kWorkspaceRoutePath),
        false,
        QStringLiteral("dismissEmbeddedOnboarding"));
}

void OnboardingRouteBootstrapController::setEmbeddedOnboardingEnabled(const bool enabled)
{
    if (m_embeddedOnboardingEnabled == enabled)
    {
        return;
    }

    m_embeddedOnboardingEnabled = enabled;
    emit embeddedOnboardingEnabledChanged();
}

void OnboardingRouteBootstrapController::setEmbeddedOnboardingVisible(const bool visible)
{
    if (m_embeddedOnboardingVisible == visible)
    {
        return;
    }

    m_embeddedOnboardingVisible = visible;
    emit embeddedOnboardingVisibleChanged();
}

void OnboardingRouteBootstrapController::setRouteCommitPending(const bool pending)
{
    if (m_routeCommitPending == pending)
    {
        return;
    }

    m_routeCommitPending = pending;
    emit routeCommitPendingChanged();
}

void OnboardingRouteBootstrapController::setStartupRoutePath(const QString& path)
{
    const QString normalizedPath = path.trimmed().isEmpty()
                                       ? QString::fromLatin1(kWorkspaceRoutePath)
                                       : path.trimmed();
    if (m_startupRoutePath == normalizedPath)
    {
        return;
    }

    m_startupRoutePath = normalizedPath;
    emit startupRoutePathChanged();
}
