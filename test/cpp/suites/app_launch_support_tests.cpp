#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::appLaunchSupport_requiresMountedHubForStartupWorkspace()
{
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(false));
    QVERIFY(WhatSon::Runtime::Bootstrap::startupWorkspaceReady(true));
}

void WhatSonCppRegressionTests::qmlLaunchSupport_routesRootLoadingThroughLvrsAppEntry()
{
    const QString launchSupportHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp"));
    const QString launchSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.cpp"));
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(launchSupportHeader.contains(QStringLiteral("#include \"backend/runtime/appentry.h\"")));
    QVERIFY(launchSupportHeader.contains(QStringLiteral("lvrs::QmlRootLoadResult loadQmlRoot(")));
    QVERIFY(launchSupportHeader.contains(QStringLiteral("lvrs::QmlRootLoadResult loadMainWindowRoot(")));
    QVERIFY(launchSupportHeader.contains(QStringLiteral("QObject* lastRootObject(")));
    QVERIFY(launchSupportHeader.contains(QStringLiteral("lvrs::QmlWindowActivationPolicy")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("lvrs::QmlRootLoadResult loadQmlRoot(")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("lvrs::QmlRootLoadSpec rootSpec;")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("lvrs::loadQmlRootObjects(")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("rootSpec.windowActivationPolicy = activationPolicy;")));
    QVERIFY(mainSource.contains(QStringLiteral("#include \"app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp\"")));
    QVERIFY(mainSource.contains(QStringLiteral("WhatSon::Runtime::Bootstrap::loadMainWindowRoot(")));
    QVERIFY(mainSource.contains(QStringLiteral("const lvrs::QmlRootLoadResult mainWindowLoadResult")));
    QVERIFY(mainSource.contains(QStringLiteral("WhatSon::Runtime::Bootstrap::lastRootObject(mainWindowLoadResult)")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::QmlWindowActivationPolicy::ShowRaiseAndActivate")));
    QVERIFY(!mainSource.contains(QStringLiteral("engine.loadFromModule(moduleUri, typeName);")));
    QVERIFY(!mainSource.contains(QStringLiteral("window->requestActivate();")));
}

void WhatSonCppRegressionTests::foregroundServiceGate_startsSchedulerAndPermissionsAfterVisibleWorkspace()
{
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(mainSource.contains(QStringLiteral("#include \"backend/runtime/foregroundservices.h\"")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::ForegroundServiceGate foregroundServiceGate(&app);")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::QmlAppLifecycleStage::AfterWindowActivated")));
    QVERIFY(mainSource.contains(QStringLiteral("context.rootLoadResult = workspaceRootLoadResult;")));
    QVERIFY(mainSource.contains(QStringLiteral("options.requireVisibleWorkspace = true;")));
    QVERIFY(mainSource.contains(QStringLiteral("foregroundServiceGate.startOnceWhenWorkspaceVisible(")));
    QVERIFY(mainSource.contains(QStringLiteral("schedulerStart.name = QStringLiteral(\"whatson-async-scheduler\")")));
    QVERIFY(mainSource.contains(QStringLiteral("permissionBootstrap.name = QStringLiteral(\"whatson-permission-bootstrap\")")));
    QVERIFY(mainSource.contains(QStringLiteral("QTimer::singleShot(0, &permissionBootstrapper")));
    QVERIFY(mainSource.contains(QStringLiteral("startForegroundServices(workspaceMainWindowLoadResult)")));
    QVERIFY(mainSource.contains(QStringLiteral("startForegroundServices(mainWindowLoadResult)")));
    QVERIFY(!mainSource.contains(QStringLiteral("bool foregroundServicesStarted = false;")));
    QVERIFY(!mainSource.contains(QStringLiteral("startForegroundServices();")));
}

void WhatSonCppRegressionTests::startupRuntimeLoad_usesLvrsAfterFirstIdleLifecycleTask()
{
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));
    const QString coordinatorHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp"));
    const QString coordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp"));

    QVERIFY(mainSource.contains(QStringLiteral("bool scheduleStartupRuntimeLoadAfterFirstIdle(")));
    QVERIFY(mainSource.contains(QStringLiteral("const lvrs::QmlRootLoadResult& workspaceRootLoadResult")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::QmlBootstrapTask runtimeLoadTask;")));
    QVERIFY(mainSource.contains(
        QStringLiteral("runtimeLoadTask.name = QStringLiteral(\"whatson-startup-runtime-load\")")));
    QVERIFY(mainSource.contains(
        QStringLiteral("runtimeLoadTask.stage = lvrs::QmlAppLifecycleStage::AfterFirstIdle;")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::scheduleQmlAppLifecycleStage(")));
    QVERIFY(mainSource.contains(
        QStringLiteral("scheduleStartupRuntimeLoadAfterFirstIdle(")));
    QVERIFY(mainSource.contains(QStringLiteral("startupRuntimeCoordinator.loadHubIntoRuntime(")));

    QVERIFY(!mainSource.contains(QStringLiteral("loadStartupHubIntoRuntime(")));
    QVERIFY(!coordinatorHeader.contains(QStringLiteral("loadStartupHubIntoRuntime")));
    QVERIFY(!coordinatorHeader.contains(QStringLiteral("ensureDeferredStartupHierarchyLoaded")));
    QVERIFY(!coordinatorHeader.contains(QStringLiteral("bindSidebarActivation")));
    QVERIFY(!coordinatorSource.contains(QStringLiteral("ensureDeferredStartupHierarchyLoaded")));
    QVERIFY(!coordinatorSource.contains(QStringLiteral("startupDeferredBootstrapActive")));

    QVERIFY(!mainSource.contains(QStringLiteral("deferredStartupHierarchyCursor")));
    QVERIFY(!mainSource.contains(QStringLiteral("scheduleDeferredStartupHierarchyLoad")));
    QVERIFY(!mainSource.contains(QStringLiteral("startup-idle-prefetch")));
    QVERIFY(!mainSource.contains(QStringLiteral("whatson-deferred-startup-hierarchy-prefetch")));
}
