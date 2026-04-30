#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class IOnboardingHubController;

class OnboardingRouteBootstrapController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool embeddedOnboardingEnabled READ embeddedOnboardingEnabled NOTIFY embeddedOnboardingEnabledChanged)
    Q_PROPERTY(bool embeddedOnboardingVisible READ embeddedOnboardingVisible NOTIFY embeddedOnboardingVisibleChanged)
    Q_PROPERTY(bool routeCommitPending READ routeCommitPending NOTIFY routeCommitPendingChanged)
    Q_PROPERTY(QString startupRoutePath READ startupRoutePath NOTIFY startupRoutePathChanged)

public:
    explicit OnboardingRouteBootstrapController(QObject* parent = nullptr);

    [[nodiscard]] bool embeddedOnboardingEnabled() const noexcept;
    [[nodiscard]] bool embeddedOnboardingVisible() const noexcept;
    [[nodiscard]] bool routeCommitPending() const noexcept;
    [[nodiscard]] QString startupRoutePath() const;

    void setHubController(IOnboardingHubController* controller);
    void configure(bool embeddedEnabled, bool startupWorkspaceReady);

    Q_INVOKABLE void handleHubLoaded();
    Q_INVOKABLE void handleOperationFailed(const QString& message);
    Q_INVOKABLE void handlePageStackNavigated(const QString& path);
    Q_INVOKABLE void handlePageStackNavigationFailed(const QString& path);
    Q_INVOKABLE void reopenEmbeddedOnboarding();
    Q_INVOKABLE void dismissEmbeddedOnboarding();

signals:
    void embeddedOnboardingEnabledChanged();
    void embeddedOnboardingVisibleChanged();
    void routeCommitPendingChanged();
    void startupRoutePathChanged();
    void routeSyncRequested(const QString& targetPath, bool deferred, const QString& reason);

private:
    void setEmbeddedOnboardingEnabled(bool enabled);
    void setEmbeddedOnboardingVisible(bool visible);
    void setRouteCommitPending(bool pending);
    void setStartupRoutePath(const QString& path);

    QPointer<IOnboardingHubController> m_hubController;
    bool m_embeddedOnboardingEnabled = false;
    bool m_embeddedOnboardingVisible = false;
    bool m_routeCommitPending = false;
    QString m_startupRoutePath = QStringLiteral("/");
};
