#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::appLaunchSupport_requiresMountedAndLoadedHubForStartupWorkspace()
{
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(false, false));
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(false, true));
    QVERIFY(!WhatSon::Runtime::Bootstrap::startupWorkspaceReady(true, false));
    QVERIFY(WhatSon::Runtime::Bootstrap::startupWorkspaceReady(true, true));
}

void WhatSonCppRegressionTests::qmlLaunchSupport_routesRootLoadingThroughLvrsAppEntry()
{
    const QString launchSupportHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp"));
    const QString launchSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.cpp"));
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(launchSupportHeader.contains(QStringLiteral("#include \"backend/runtime/appentry.h\"")));
    QVERIFY(launchSupportHeader.contains(QStringLiteral("lvrs::QmlWindowActivationPolicy")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("lvrs::QmlRootLoadSpec rootSpec;")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("lvrs::loadQmlRootObjects(")));
    QVERIFY(launchSupportSource.contains(QStringLiteral("rootSpec.windowActivationPolicy = activationPolicy;")));
    QVERIFY(mainSource.contains(QStringLiteral("#include \"app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp\"")));
    QVERIFY(mainSource.contains(QStringLiteral("WhatSon::Runtime::Bootstrap::loadMainWindow(")));
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
    QVERIFY(mainSource.contains(QStringLiteral("context.rootLoadResult.windows.append(workspaceWindow);")));
    QVERIFY(mainSource.contains(QStringLiteral("options.requireVisibleWorkspace = true;")));
    QVERIFY(mainSource.contains(QStringLiteral("foregroundServiceGate.startOnceWhenWorkspaceVisible(")));
    QVERIFY(mainSource.contains(QStringLiteral("schedulerStart.name = QStringLiteral(\"whatson-async-scheduler\")")));
    QVERIFY(mainSource.contains(QStringLiteral("permissionBootstrap.name = QStringLiteral(\"whatson-permission-bootstrap\")")));
    QVERIFY(mainSource.contains(QStringLiteral("QTimer::singleShot(0, &permissionBootstrapper")));
    QVERIFY(mainSource.contains(QStringLiteral("startForegroundServices(workspaceMainWindow)")));
    QVERIFY(mainSource.contains(QStringLiteral("startForegroundServices(mainWindow)")));
    QVERIFY(!mainSource.contains(QStringLiteral("bool foregroundServicesStarted = false;")));
    QVERIFY(!mainSource.contains(QStringLiteral("startForegroundServices();")));
}

void WhatSonCppRegressionTests::startupDeferredHierarchyPrefetch_usesLvrsAfterFirstIdleLifecycleTask()
{
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));
    const QString coordinatorHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp"));
    const QString coordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp"));

    QVERIFY(mainSource.contains(QStringLiteral("bool scheduleDeferredStartupHierarchyPrefetch(")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::QmlBootstrapTask prefetchTask;")));
    QVERIFY(mainSource.contains(
        QStringLiteral("prefetchTask.name = QStringLiteral(\"whatson-deferred-startup-hierarchy-prefetch\")")));
    QVERIFY(mainSource.contains(
        QStringLiteral("prefetchTask.stage = lvrs::QmlAppLifecycleStage::AfterFirstIdle;")));
    QVERIFY(mainSource.contains(QStringLiteral("lvrs::scheduleQmlAppLifecycleStage(")));
    QVERIFY(mainSource.contains(QStringLiteral("startup-after-first-idle-prefetch")));
    QVERIFY(mainSource.contains(
        QStringLiteral("scheduleDeferredStartupHierarchyPrefetch(app, engine, mainWindow, startupRuntimeCoordinator)")));

    QVERIFY(coordinatorHeader.contains(
        QStringLiteral("bool ensureDeferredStartupHierarchyLoaded(int hierarchyIndex, const QString& reason);")));
    QVERIFY(coordinatorSource.contains(
        QStringLiteral("bool WhatSonStartupRuntimeCoordinator::ensureDeferredStartupHierarchyLoaded(")));
    QVERIFY(coordinatorSource.contains(QStringLiteral("return false;")));

    QVERIFY(!mainSource.contains(QStringLiteral("deferredStartupHierarchyCursor")));
    QVERIFY(!mainSource.contains(QStringLiteral("scheduleDeferredStartupHierarchyLoad")));
    QVERIFY(!mainSource.contains(QStringLiteral("startup-idle-prefetch")));
}
