#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle()
{
    OnboardingRouteBootstrapController controller;
    FakeOnboardingHubController hubController;
    controller.setHubController(&hubController);

    QSignalSpy routeSyncSpy(
        &controller,
        &OnboardingRouteBootstrapController::routeSyncRequested);

    controller.configure(true, false);

    QVERIFY(controller.embeddedOnboardingEnabled());
    QVERIFY(controller.embeddedOnboardingVisible());
    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/onboarding"));

    controller.handleHubLoaded();
    QCOMPARE(hubController.beginCount, 1);
    QVERIFY(controller.routeCommitPending());
    QVERIFY(!controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 1);
    QCOMPARE(routeSyncSpy.at(0).at(0).toString(), QStringLiteral("/"));
    QCOMPARE(routeSyncSpy.at(0).at(1).toBool(), true);
    QCOMPARE(routeSyncSpy.at(0).at(2).toString(), QStringLiteral("embeddedHubLoaded"));

    controller.handleOperationFailed(QStringLiteral("load failed"));
    QCOMPARE(hubController.failCount, 1);
    QCOMPARE(hubController.lastFailureMessage, QStringLiteral("load failed"));
    QVERIFY(!controller.routeCommitPending());
    QVERIFY(controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 2);
    QCOMPARE(routeSyncSpy.at(1).at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(routeSyncSpy.at(1).at(1).toBool(), true);
    QCOMPARE(routeSyncSpy.at(1).at(2).toString(), QStringLiteral("embeddedOperationFailed"));

    controller.handleHubLoaded();
    controller.handlePageStackNavigated(QStringLiteral("/"));
    QCOMPARE(hubController.beginCount, 2);
    QCOMPARE(hubController.completeCount, 1);
    QVERIFY(!controller.routeCommitPending());

    controller.handleHubLoaded();
    controller.handlePageStackNavigationFailed(QStringLiteral("/workspace"));
    QCOMPARE(hubController.failCount, 2);
    QCOMPARE(
        hubController.lastFailureMessage,
        QStringLiteral("Failed to open the WhatSon workspace after onboarding."));
    QVERIFY(!controller.routeCommitPending());
    QVERIFY(controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 5);
    QCOMPARE(routeSyncSpy.at(4).at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(routeSyncSpy.at(4).at(1).toBool(), false);
    QCOMPARE(routeSyncSpy.at(4).at(2).toString(), QStringLiteral("navigationFailed"));

    controller.dismissEmbeddedOnboarding();
    QVERIFY(!controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 6);
    QCOMPARE(routeSyncSpy.at(5).at(0).toString(), QStringLiteral("/"));
    QCOMPARE(routeSyncSpy.at(5).at(1).toBool(), false);
    QCOMPARE(routeSyncSpy.at(5).at(2).toString(), QStringLiteral("dismissEmbeddedOnboarding"));

    controller.reopenEmbeddedOnboarding();
    QVERIFY(controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 7);
    QCOMPARE(routeSyncSpy.at(6).at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(routeSyncSpy.at(6).at(1).toBool(), false);
    QCOMPARE(routeSyncSpy.at(6).at(2).toString(), QStringLiteral("reopenEmbeddedOnboarding"));

    controller.configure(true, true);
    QVERIFY(controller.embeddedOnboardingEnabled());
    QVERIFY(!controller.embeddedOnboardingVisible());
    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/"));

    controller.configure(false, false);
    QVERIFY(!controller.embeddedOnboardingEnabled());
    QVERIFY(!controller.embeddedOnboardingVisible());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/"));
}
