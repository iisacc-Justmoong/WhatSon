#include "OnboardingRouteBootstrapController.hpp"

#include "OnboardingHubController.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

class OnboardingRouteBootstrapControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void configure_embeddedStartupWithoutHub_usesOnboardingRoute();
    void configure_workspaceStartupWhenHubLoaded_usesWorkspaceRoute();
    void handleHubLoaded_requestsDeferredWorkspaceRoute();
    void handlePageStackNavigated_completesPendingTransition();
    void handlePageStackNavigationFailed_restoresOnboardingRoute();
};

void OnboardingRouteBootstrapControllerTest::configure_embeddedStartupWithoutHub_usesOnboardingRoute()
{
    OnboardingRouteBootstrapController controller;

    controller.configure(true, false);

    QVERIFY(controller.embeddedOnboardingEnabled());
    QVERIFY(controller.embeddedOnboardingVisible());
    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/onboarding"));
}

void OnboardingRouteBootstrapControllerTest::configure_workspaceStartupWhenHubLoaded_usesWorkspaceRoute()
{
    OnboardingRouteBootstrapController controller;

    controller.configure(true, true);

    QVERIFY(controller.embeddedOnboardingEnabled());
    QVERIFY(!controller.embeddedOnboardingVisible());
    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/"));
}

void OnboardingRouteBootstrapControllerTest::handleHubLoaded_requestsDeferredWorkspaceRoute()
{
    OnboardingHubController hubController;
    OnboardingRouteBootstrapController controller;
    controller.setHubController(&hubController);
    controller.configure(true, false);

    QSignalSpy routeSyncRequestedSpy(&controller, &OnboardingRouteBootstrapController::routeSyncRequested);

    controller.handleHubLoaded();

    QVERIFY(!controller.embeddedOnboardingVisible());
    QVERIFY(controller.routeCommitPending());
    QCOMPARE(hubController.sessionState(), QStringLiteral("routingWorkspace"));
    QCOMPARE(routeSyncRequestedSpy.count(), 1);
    const QList<QVariant> firstEmission = routeSyncRequestedSpy.takeFirst();
    QCOMPARE(firstEmission.at(0).toString(), QStringLiteral("/"));
    QCOMPARE(firstEmission.at(1).toBool(), true);
    QCOMPARE(firstEmission.at(2).toString(), QStringLiteral("embeddedHubLoaded"));
}

void OnboardingRouteBootstrapControllerTest::handlePageStackNavigated_completesPendingTransition()
{
    OnboardingHubController hubController;
    OnboardingRouteBootstrapController controller;
    controller.setHubController(&hubController);
    controller.configure(true, false);
    controller.handleHubLoaded();

    controller.handlePageStackNavigated(QStringLiteral("/"));

    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(hubController.sessionState(), QStringLiteral("ready"));
}

void OnboardingRouteBootstrapControllerTest::handlePageStackNavigationFailed_restoresOnboardingRoute()
{
    OnboardingHubController hubController;
    OnboardingRouteBootstrapController controller;
    controller.setHubController(&hubController);
    controller.configure(true, false);
    QSignalSpy routeSyncRequestedSpy(&controller, &OnboardingRouteBootstrapController::routeSyncRequested);
    controller.handleHubLoaded();
    controller.handlePageStackNavigationFailed(QStringLiteral("/"));

    QVERIFY(controller.embeddedOnboardingVisible());
    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(hubController.sessionState(), QStringLiteral("failed"));
    QCOMPARE(routeSyncRequestedSpy.count(), 2);
    const QList<QVariant> firstEmission = routeSyncRequestedSpy.takeFirst();
    QCOMPARE(firstEmission.at(0).toString(), QStringLiteral("/"));
    QCOMPARE(firstEmission.at(1).toBool(), true);
    QCOMPARE(firstEmission.at(2).toString(), QStringLiteral("embeddedHubLoaded"));
    const QList<QVariant> secondEmission = routeSyncRequestedSpy.takeFirst();
    QCOMPARE(secondEmission.at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(secondEmission.at(1).toBool(), false);
    QCOMPARE(secondEmission.at(2).toString(), QStringLiteral("navigationFailed"));
}

QTEST_MAIN(OnboardingRouteBootstrapControllerTest)

#include "test_onboarding_route_bootstrap_controller.moc"
