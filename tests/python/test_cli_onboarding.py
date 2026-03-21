from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class CliOnboardingTests(unittest.TestCase):
    def test_rust_cli_supports_onboard_command(self) -> None:
        cli_text = (REPO_ROOT / "src/cli/src/main.rs").read_text(encoding="utf-8")

        self.assertIn('Usage: whatson [onboard]', cli_text)
        self.assertIn('command == "onboard"', cli_text)
        self.assertIn('LaunchMode::OnboardingOnly', cli_text)
        self.assertIn('"--onboarding-only"', cli_text)
        self.assertIn('.arg("WhatSon")', cli_text)

    def test_app_supports_onboarding_only_launch_mode(self) -> None:
        app_main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        main_qml_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        onboarding_qml_text = (REPO_ROOT / "src/app/qml/window/Onboarding.qml").read_text(encoding="utf-8")
        onboarding_content_text = (
                REPO_ROOT / "src/app/qml/window/OnboardingContent.qml"
        ).read_text(encoding="utf-8")
        creator_header_text = (
                REPO_ROOT / "src/app/file/hub/WhatSonHubCreator.hpp"
        ).read_text(encoding="utf-8")
        controller_header_text = (
                REPO_ROOT / "src/app/viewmodel/onboarding/OnboardingHubController.hpp"
        ).read_text(encoding="utf-8")
        controller_text = (
                REPO_ROOT / "src/app/viewmodel/onboarding/OnboardingHubController.cpp"
        ).read_text(encoding="utf-8")
        route_bootstrap_header_text = (
                REPO_ROOT / "src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"
        ).read_text(encoding="utf-8")
        route_bootstrap_impl_text = (
                REPO_ROOT / "src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp"
        ).read_text(encoding="utf-8")
        hub_path_utils_text = (
                REPO_ROOT / "src/app/file/hub/WhatSonHubPathUtils.hpp"
        ).read_text(encoding="utf-8")
        android_storage_header_text = (
                REPO_ROOT / "src/app/platform/Android/WhatSonAndroidStorageBackend.hpp"
        ).read_text(encoding="utf-8")
        android_storage_impl_text = (
                REPO_ROOT / "src/app/platform/Android/WhatSonAndroidStorageBackend.cpp"
        ).read_text(encoding="utf-8")
        android_build_gradle_text = (
                REPO_ROOT / "platform/Android/build.gradle"
        ).read_text(encoding="utf-8")
        ios_access_header_text = (
                REPO_ROOT / "src/app/platform/Apple/AppleSecurityScopedResourceAccess.hpp"
        ).read_text(encoding="utf-8")
        ios_access_impl_text = (
                REPO_ROOT / "src/app/platform/Apple/AppleSecurityScopedResourceAccess.mm"
        ).read_text(encoding="utf-8")
        selected_hub_store_text = (
                REPO_ROOT / "src/app/store/hub/SelectedHubStore.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("#include <QCommandLineParser>", app_main_text)
        self.assertIn('#include "backend/runtime/appbootstrap.h"', app_main_text)
        self.assertIn('QStringLiteral("onboarding-only")', app_main_text)
        self.assertIn("engine.loadFromModule(moduleUri, typeName);", app_main_text)
        self.assertIn("lvrs::AppBootstrapOptions bootstrapOptions;", app_main_text)
        self.assertIn("const lvrs::AppBootstrapState bootstrapState = lvrs::preApplicationBootstrap(bootstrapOptions);",
                      app_main_text)
        self.assertIn("lvrs::postApplicationBootstrap(app, bootstrapOptions);", app_main_text)
        self.assertIn(
            'QObject* onboardingWindow = loadWindowFromModule(',
            app_main_text,
        )
        self.assertIn(
            'QStringLiteral("WhatSon.App"),',
            app_main_text,
        )
        self.assertIn(
            'QStringLiteral("Onboarding"),',
            app_main_text,
        )
        self.assertIn(
            'QStringLiteral("Main"),',
            app_main_text,
        )
        self.assertNotIn("mobileStandaloneOnboarding", app_main_text)
        self.assertIn("if (launchOptions.onboardingOnly)", app_main_text)
        self.assertIn("const auto loadWindowFromModule =", app_main_text)
        self.assertIn("engine.setInitialProperties(initialProperties);", app_main_text)
        self.assertIn("engine.setInitialProperties({});", app_main_text)
        self.assertIn("const QVariantMap onboardingWindowInitialProperties{", app_main_text)
        self.assertIn("const QVariantMap mainWindowInitialProperties{", app_main_text)
        self.assertIn('QStringLiteral("standaloneMode"), true', app_main_text)
        self.assertIn('QStringLiteral("visible"), true', app_main_text)
        self.assertIn('#include "viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"', app_main_text)
        self.assertIn("OnboardingRouteBootstrapController onboardingRouteBootstrapController;", app_main_text)
        self.assertIn("onboardingRouteBootstrapController.setHubController(&onboardingHubController);", app_main_text)
        self.assertIn("const bool useEmbeddedStartupOnboarding = true;", app_main_text)
        self.assertIn("bool startupHubMounted = false;", app_main_text)
        self.assertIn("const QString blueprintFallbackHubPath = resolveBlueprintHubPath();", app_main_text)
        self.assertIn("const QString startupHubSelectionPath = selectedHubStore.startupHubPath(blueprintFallbackHubPath);", app_main_text)
        self.assertIn("const auto resolveStartupHubMountPath = [](const QString& hubPath, QString* errorMessage) -> QString", app_main_text)
        self.assertIn("const auto tryResolveStartupHubSelection =", app_main_text)
        self.assertIn("Failed to resolve %1 startup WhatSon Hub mount '%2': %3", app_main_text)
        self.assertIn('QStringLiteral("persisted")', app_main_text)
        self.assertIn('QStringLiteral("blueprint fallback")', app_main_text)
        self.assertIn("WhatSon::HubPath::normalizePath(startupHubSelectionPath)", app_main_text)
        self.assertIn("WhatSon::HubPath::normalizePath(blueprintFallbackHubPath)", app_main_text)
        self.assertIn("const bool showDesktopStartupOnboarding = !startupHubMounted && !useEmbeddedStartupOnboarding;", app_main_text)
        self.assertIn("onboardingRouteBootstrapController.configure(useEmbeddedStartupOnboarding, startupHubMounted);", app_main_text)
        self.assertIn("onboardingRouteBootstrapController.configure(false, true);", app_main_text)
        self.assertNotIn('onboardingWindow->setProperty("standaloneMode", true);', app_main_text)
        self.assertNotIn('mainWindow->setProperty("onboardingVisible", true);', app_main_text)
        self.assertIn('"hubSessionController"', app_main_text)
        self.assertIn('"onboardingHubController"', app_main_text)
        self.assertIn('"onboardingRouteBootstrapController"', app_main_text)
        self.assertIn('"desktopOnboardingWindowVisible"', app_main_text)
        self.assertIn('#include "platform/Android/WhatSonAndroidStorageBackend.hpp"', app_main_text)
        self.assertIn("onboardingHubController.syncCurrentHubSelection(startupHubPath);", app_main_text)
        self.assertIn("SelectedHubStore selectedHubStore;", app_main_text)
        self.assertIn("selectedHubStore.startupHubPath(blueprintFallbackHubPath)", app_main_text)
        self.assertIn("selectedHubStore.setSelectedHubPath(hubPath);", app_main_text)
        self.assertIn("WhatSon::Android::Storage::isSupportedUri(normalizedHubPath)", app_main_text)
        self.assertIn("WhatSon::Android::Storage::isMountedHubPath(normalizedHubPath)", app_main_text)
        self.assertIn("WhatSon::Android::Storage::mountedHubSourceUri(normalizedHubPath);", app_main_text)
        self.assertIn("WhatSon::Android::Storage::mountHub(normalizedHubPath, &mountedHubPath, errorMessage)", app_main_text)
        self.assertIn("WhatSon::Android::Storage::mountHub(sourceUri, &remountedHubPath, errorMessage)", app_main_text)
        self.assertIn("#include <QWindow>", app_main_text)
        self.assertIn("const auto activateWindowObject = [](QObject* windowObject)", app_main_text)
        self.assertIn("if (auto* window = qobject_cast<QWindow*>(windowObject))", app_main_text)
        self.assertIn("window->show();", app_main_text)
        self.assertIn("window->requestActivate();", app_main_text)
        self.assertIn("const QMetaObject::Connection onboardingDismissedConnection = QObject::connect(", app_main_text)
        self.assertIn("SIGNAL(dismissed())", app_main_text)
        self.assertIn("SLOT(quit())", app_main_text)
        self.assertIn("&loadMainWindow", app_main_text)
        self.assertIn("&onboardingRouteBootstrapController", app_main_text)
        self.assertIn("workspaceMainWindow = loadMainWindow(mainWindowInitialProperties);", app_main_text)
        self.assertIn("QObject* mainWindow = loadMainWindow(mainWindowInitialProperties);", app_main_text)
        self.assertIn("QObject::disconnect(onboardingDismissedConnection);", app_main_text)
        self.assertIn("&activateWindowObject", app_main_text)
        self.assertIn("activateWindowObject(workspaceMainWindow);", app_main_text)
        self.assertIn("activateWindowObject(mainWindow);", app_main_text)
        self.assertIn("QTimer::singleShot(0, &app, [onboardingWindowGuard]()", app_main_text)
        self.assertNotIn("onboardingWindowGuard->deleteLater();", app_main_text)
        load_main_index = app_main_text.find("workspaceMainWindow = loadMainWindow(mainWindowInitialProperties);")
        activate_main_index = app_main_text.find("activateWindowObject(workspaceMainWindow);")
        hide_onboarding_index = app_main_text.find('onboardingWindowGuard->setProperty("visible", false);')
        self.assertGreaterEqual(load_main_index, 0)
        self.assertGreaterEqual(activate_main_index, 0)
        self.assertGreaterEqual(hide_onboarding_index, 0)
        self.assertLess(load_main_index, hide_onboarding_index)
        self.assertLess(activate_main_index, hide_onboarding_index)
        self.assertIn("readonly property int onboardingMinHeight: 420", main_qml_text)
        self.assertIn("readonly property int onboardingMinWidth: 620", main_qml_text)
        self.assertIn(
            "readonly property int onboardingDefaultHeight: onboardingMinHeight",
            main_qml_text,
        )
        self.assertIn(
            "readonly property int onboardingDefaultWidth: onboardingMinWidth",
            main_qml_text,
        )
        self.assertIn('readonly property string onboardingRoutePath: "/onboarding"', main_qml_text)
        self.assertIn("readonly property bool useEmbeddedOnboardingRoute: adaptiveMobileLayout || isMobilePlatform", main_qml_text)
        self.assertIn("property bool desktopOnboardingWindowVisible: false", main_qml_text)
        self.assertIn("property var onboardingRouteBootstrapController: null", main_qml_text)
        self.assertIn("readonly property string startupRoutePath: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.startupRoutePath : workspaceRoutePath", main_qml_text)
        self.assertIn("readonly property bool onboardingRouteCommitPending: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.routeCommitPending : false", main_qml_text)
        self.assertIn("readonly property color mobileSafeAreaBackdropColor: canvasColor", main_qml_text)
        self.assertIn("mobileOversizedHeightEnabled: false", main_qml_text)
        self.assertIn("forcedDeviceTierPreset: -1", main_qml_text)
        self.assertIn("function applyRequestedRoute(targetPath, routeSource)", main_qml_text)
        self.assertIn("applicationWindow.activePageRouter.setRoot(targetPath);", main_qml_text)
        self.assertIn("initialRoutePath: startupRoutePath", main_qml_text)
        self.assertIn("pageInitialPath: startupRoutePath", main_qml_text)
        self.assertNotIn("pageInitialPath: applicationWindow.useEmbeddedOnboardingRoute && applicationWindow.onboardingVisible ? onboardingRoutePath : workspaceRoutePath", main_qml_text)
        self.assertIn("pageRoutes: [onboardingRoute, workspaceShellRoute]", main_qml_text)
        self.assertIn("property var onboardingHubController: null", main_qml_text)
        self.assertIn("hubSessionController: applicationWindow.onboardingHubController", main_qml_text)
        self.assertIn("WindowView.OnboardingContent {", main_qml_text)
        self.assertIn("autoCompleteOnHubLoaded: false", main_qml_text)
        self.assertIn("target: applicationWindow.onboardingRouteBootstrapController", main_qml_text)
        self.assertIn("target: applicationWindow.onboardingHubController", main_qml_text)
        self.assertIn("id: mobileSafeAreaColorOverride", main_qml_text)
        self.assertIn("parent: applicationWindow.nativeWindowContentRoot", main_qml_text)
        self.assertIn("!applicationWindow.fullWindowAreaOnMobileEnabled", main_qml_text)
        self.assertIn("color: applicationWindow.mobileSafeAreaBackdropColor", main_qml_text)
        self.assertIn('source: applicationWindow.platform === "osx"', main_qml_text)
        self.assertIn('Qt.resolvedUrl("window/MacNativeMenuBar.qml")', main_qml_text)
        self.assertIn("item.hostWindow = applicationWindow", main_qml_text)
        self.assertNotIn("WindowView.MacNativeMenuBar {", main_qml_text)
        self.assertIn("applicationWindow.onboardingRouteBootstrapController.reopenEmbeddedOnboarding();", main_qml_text)
        self.assertIn("applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigated(String(path));", main_qml_text)
        self.assertIn("applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigationFailed(String(path));", main_qml_text)
        self.assertIn("applicationWindow.onboardingRouteBootstrapController.handleHubLoaded();", main_qml_text)
        self.assertIn("applicationWindow.onboardingRouteBootstrapController.handleOperationFailed(message);", main_qml_text)
        self.assertIn("applicationWindow.onboardingRouteBootstrapController.dismissEmbeddedOnboarding();", main_qml_text)
        self.assertIn("applicationWindow.applyRequestedRoute(String(targetPath), String(reason));", main_qml_text)
        self.assertIn("applicationWindow.applyRequestedRoute(applicationWindow.startupRoutePath, \"completed\");", main_qml_text)
        self.assertIn("readonly property int desktopMinHeight: 420", onboarding_qml_text)
        self.assertIn("readonly property int desktopMinWidth: 620", onboarding_qml_text)
        self.assertIn('import "." as WindowView', onboarding_qml_text)
        self.assertIn("WindowView.OnboardingContent {", onboarding_qml_text)
        self.assertIn("onDismissRequested: root.close()", onboarding_qml_text)
        self.assertIn("onCompleted: root.close()", onboarding_qml_text)
        self.assertIn("onRequestWindowMove:", onboarding_qml_text)
        self.assertIn("typeof root.startSystemMove === \"function\"", onboarding_qml_text)
        self.assertIn("property var hostWindow: null", onboarding_qml_text)
        self.assertIn("property bool standaloneMode: false", onboarding_qml_text)
        self.assertIn("property string versionText: \"Version: 1.0.0\"", onboarding_qml_text)
        self.assertIn(
            "readonly property bool hasHubSelectionCandidates: root.hubSessionController && root.hubSessionController.hubSelectionCandidateNames.length > 0",
            onboarding_content_text,
        )
        self.assertIn("readonly property string onboardingSessionState:", onboarding_content_text)
        self.assertIn("property bool autoCompleteOnHubLoaded: true", onboarding_content_text)
        self.assertIn('readonly property bool useAndroidExistingHubFileFlow: Qt.platform.os === "android"', onboarding_content_text)
        self.assertIn("readonly property bool useMobileCreateDirectoryFlow: root.isMobilePlatform", onboarding_content_text)
        self.assertIn("readonly property int mobileActionSpacing: 24", onboarding_content_text)
        self.assertIn("readonly property int mobileActionWidth: 180", onboarding_content_text)
        self.assertIn("readonly property int mobileContentSpacing: 32", onboarding_content_text)
        self.assertIn("readonly property int mobileContentWidth: 209", onboarding_content_text)
        self.assertIn("readonly property int mobileDesignHeight: 762", onboarding_content_text)
        self.assertIn("readonly property int mobileDesignWidth: 470", onboarding_content_text)
        self.assertIn("readonly property int mobileSurfaceRadius: 32", onboarding_content_text)
        self.assertIn("readonly property int mobileVersionWidth: 75", onboarding_content_text)
        self.assertIn("readonly property bool useMobileLayout:", onboarding_content_text)
        self.assertIn(
            "readonly property int mobileWindowHeight: Math.max(0, root.availableScreenHeight)",
            onboarding_qml_text,
        )
        self.assertIn(
            "readonly property int mobileWindowWidth: Math.max(0, root.availableScreenWidth)",
            onboarding_qml_text,
        )
        self.assertIn(
            "readonly property int compactMinHeight: root.useMobileLayout ? root.mobileWindowHeight : root.desktopMinHeight",
            onboarding_qml_text,
        )
        self.assertIn(
            "readonly property int compactMinWidth: root.useMobileLayout ? root.mobileWindowWidth : root.desktopMinWidth",
            onboarding_qml_text,
        )
        self.assertIn("readonly property int dragRegionHeight: 72", onboarding_content_text)
        self.assertIn("cursorShape: Qt.SizeAllCursor", onboarding_content_text)
        self.assertIn("readonly property int fixedHeight: Math.max(defaultHeight, minHeight)", onboarding_qml_text)
        self.assertIn("readonly property int fixedWidth: Math.max(defaultWidth, minWidth)", onboarding_qml_text)
        self.assertIn(
            "readonly property int effectiveHeight: root.useMobileLayout ? root.compactMinHeight : root.fixedHeight",
            onboarding_qml_text,
        )
        self.assertIn(
            "readonly property int effectiveWidth: root.useMobileLayout ? root.compactMinWidth : root.fixedWidth",
            onboarding_qml_text,
        )
        self.assertIn("property int defaultHeight: compactMinHeight", onboarding_qml_text)
        self.assertIn("property int defaultWidth: compactMinWidth", onboarding_qml_text)
        self.assertIn("height: effectiveHeight", onboarding_qml_text)
        self.assertIn("width: effectiveWidth", onboarding_qml_text)
        self.assertIn("minimumHeight: effectiveHeight", onboarding_qml_text)
        self.assertIn("minimumWidth: effectiveWidth", onboarding_qml_text)
        self.assertIn("maximumHeight: effectiveHeight", onboarding_qml_text)
        self.assertIn("maximumWidth: effectiveWidth", onboarding_qml_text)
        self.assertIn("var targetScreen = root.screen;", onboarding_qml_text)
        self.assertIn("radius: root.useMobileLayout ? root.mobileSurfaceRadius : 32", onboarding_content_text)
        self.assertIn("radius: windowFrame.radius", onboarding_content_text)
        self.assertIn("anchors.rightMargin: windowFrame.radius", onboarding_content_text)
        self.assertIn("import QtQuick.Dialogs", onboarding_content_text)
        self.assertIn("FileDialog {", onboarding_content_text)
        self.assertIn('readonly property string defaultCreateHubFileName: "Untitled.wshub"', onboarding_content_text)
        self.assertIn("readonly property url suggestedCreateHubFileUrl:", onboarding_content_text)
        self.assertIn("currentFile: root.suggestedCreateHubFileUrl", onboarding_content_text)
        self.assertIn("selectedFile: root.suggestedCreateHubFileUrl", onboarding_content_text)
        self.assertIn("id: createHubDirectoryDialog", onboarding_content_text)
        self.assertIn('title: "Choose Folder for New WhatSon Hub"', onboarding_content_text)
        self.assertIn("FolderDialog {", onboarding_content_text)
        self.assertIn("createHubInDirectoryUrl(selectedFolder, root.defaultCreateHubFileName)", onboarding_content_text)
        self.assertIn("createHubAtUrl(selectedFile)", onboarding_content_text)
        self.assertIn("id: selectHubFileDialog", onboarding_content_text)
        self.assertIn("fileMode: FileDialog.OpenFile", onboarding_content_text)
        self.assertIn('nameFilters: ["WhatSon Hub (*.wshub)"]', onboarding_content_text)
        self.assertIn("root.hubSessionController.loadHubFromUrl(selectedFile);", onboarding_content_text)
        self.assertIn('title: root.isMobilePlatform ? "Choose Folder Containing WhatSon Hub" : "Select WhatSon Hub"',
                      onboarding_content_text)
        self.assertIn("prepareHubSelectionFromUrl(selectedFolder)", onboarding_content_text)
        self.assertIn("loadHubFromUrl(selectedFolder)", onboarding_content_text)
        self.assertIn("if (root.useAndroidExistingHubFileFlow)", onboarding_content_text)
        self.assertIn("selectHubFileDialog.open();", onboarding_content_text)
        self.assertIn("loadHubSelectionCandidate(index)", onboarding_content_text)
        self.assertIn("readonly property string mobileSelectionAssistText:", onboarding_content_text)
        self.assertIn("On Android, choose the .wshub package directly from the native picker.", onboarding_content_text)
        self.assertIn("Choose the WhatSon Hub package found in the selected folder.", onboarding_content_text)
        self.assertIn("On mobile, choose the folder that contains your WhatSon Hub.", onboarding_content_text)
        self.assertIn("root.hubSessionController.clearHubSelectionCandidates();", onboarding_content_text)
        self.assertIn("root.hubSessionController.hubSelectionCandidateNames", onboarding_content_text)
        self.assertIn("Q_INVOKABLE bool createHubInDirectoryUrl(const QUrl& directoryUrl, const QString& preferredFileName);",
                      controller_header_text)
        self.assertIn(
            "Q_PROPERTY(QStringList hubSelectionCandidateNames READ hubSelectionCandidateNames NOTIFY hubSelectionCandidatesChanged)",
            controller_header_text,
        )
        self.assertIn("Q_PROPERTY(QString sessionState READ sessionState NOTIFY sessionStateChanged)", controller_header_text)
        self.assertIn("Q_INVOKABLE bool prepareHubSelectionFromUrl(const QUrl& hubUrl);", controller_header_text)
        self.assertIn("Q_INVOKABLE bool loadHubSelectionCandidate(int index);", controller_header_text)
        self.assertIn("Q_INVOKABLE void beginWorkspaceTransition();", controller_header_text)
        self.assertIn("Q_INVOKABLE void completeWorkspaceTransition();", controller_header_text)
        self.assertIn("Q_INVOKABLE void failWorkspaceTransition(const QString& message);", controller_header_text)
        self.assertIn("void clearHubSelectionCandidates();", controller_header_text)
        self.assertIn("void sessionStateChanged();", controller_header_text)
        self.assertIn('QString m_sessionState = QStringLiteral("idle");', controller_header_text)
        self.assertIn('#include "file/hub/WhatSonHubPathUtils.hpp"', controller_text)
        self.assertIn('#include "platform/Android/WhatSonAndroidStorageBackend.hpp"', controller_text)
        self.assertIn('#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"', controller_text)
        self.assertIn("#include <QTemporaryDir>", controller_text)
        self.assertIn("#include <exception>", controller_text)
        self.assertIn('constexpr auto kDefaultMobileHubFileName = "Untitled.wshub";', controller_text)
        self.assertIn('constexpr auto kSessionStateRoutingWorkspace = "routingWorkspace";', controller_text)
        self.assertIn('constexpr auto kSessionStateReady = "ready";', controller_text)
        self.assertIn("uniqueHubPackagePathInDirectory(directoryPath, preferredFileName)", controller_text)
        self.assertIn("bool invokeCreateHubCallbackSafely(", controller_text)
        self.assertIn("bool invokeLoadHubCallbackSafely(", controller_text)
        self.assertIn("WhatSon::HubPath::joinPath(normalizedDirectoryPath, candidateName)", controller_text)
        self.assertIn("candidateName = QStringLiteral(\"%1-%2%3\")", controller_text)
        self.assertIn("bool OnboardingHubController::prepareHubSelectionFromUrl(const QUrl& hubUrl)", controller_text)
        self.assertIn("const QStringList packageCandidates = hubPackageCandidatesInDirectory(normalizedSelectedPath);",
                      controller_text)
        self.assertIn("WhatSon::Android::Storage::resolveHubSelection(", controller_text)
        self.assertIn("setHubSelectionCandidatePaths(packageCandidates);", controller_text)
        self.assertIn("bool OnboardingHubController::loadHubSelectionCandidate(int index)", controller_text)
        self.assertIn("bool OnboardingHubController::validateMountableHubPath(", controller_text)
        self.assertIn("bool OnboardingHubController::loadResolvedHubPath(", controller_text)
        self.assertIn("WhatSon::Android::Storage::isSupportedUri(directoryPath)", controller_text)
        self.assertIn("QTemporaryDir temporaryDirectory;", controller_text)
        self.assertIn("WhatSon::Android::Storage::exportLocalHubToDirectory(", controller_text)
        self.assertIn("WhatSon::Android::Storage::mountHub(resolvedHubPath, &mountableHubPath, errorMessage)", controller_text)
        self.assertIn("Selected Android folder does not contain a WhatSon Hub package directory.", controller_text)
        self.assertIn("void OnboardingHubController::beginWorkspaceTransition()", controller_text)
        self.assertIn("void OnboardingHubController::completeWorkspaceTransition()", controller_text)
        self.assertIn("void OnboardingHubController::failWorkspaceTransition(const QString& message)", controller_text)
        self.assertIn('setSessionState(QString::fromLatin1(kSessionStateRoutingWorkspace));', controller_text)
        self.assertIn('setSessionState(QString::fromLatin1(kSessionStateReady));', controller_text)
        self.assertIn('setSessionState(QString::fromLatin1(kSessionStateHubLoaded));', controller_text)
        self.assertIn("void OnboardingHubController::setSessionState(const QString& sessionState)", controller_text)
        self.assertIn("class OnboardingRouteBootstrapController final : public QObject", route_bootstrap_header_text)
        self.assertIn("Q_PROPERTY(QString startupRoutePath READ startupRoutePath NOTIFY startupRoutePathChanged)",
                      route_bootstrap_header_text)
        self.assertIn("Q_PROPERTY(bool embeddedOnboardingVisible READ embeddedOnboardingVisible NOTIFY embeddedOnboardingVisibleChanged)",
                      route_bootstrap_header_text)
        self.assertIn("void configure(bool embeddedEnabled, bool startupHubMounted);", route_bootstrap_header_text)
        self.assertIn("Q_INVOKABLE void handleHubLoaded();", route_bootstrap_header_text)
        self.assertIn("Q_INVOKABLE void handlePageStackNavigated(const QString& path);", route_bootstrap_header_text)
        self.assertIn("Q_INVOKABLE void reopenEmbeddedOnboarding();", route_bootstrap_header_text)
        self.assertIn('constexpr auto kWorkspaceRoutePath = "/";', route_bootstrap_impl_text)
        self.assertIn('constexpr auto kOnboardingRoutePath = "/onboarding";', route_bootstrap_impl_text)
        self.assertIn("void OnboardingRouteBootstrapController::configure(const bool embeddedEnabled, const bool startupHubMounted)",
                      route_bootstrap_impl_text)
        self.assertIn("setEmbeddedOnboardingVisible(embeddedEnabled && !startupHubMounted);", route_bootstrap_impl_text)
        self.assertIn("m_hubController->beginWorkspaceTransition();", route_bootstrap_impl_text)
        self.assertIn("emit routeSyncRequested(", route_bootstrap_impl_text)
        self.assertIn('QStringLiteral("embeddedHubLoaded")', route_bootstrap_impl_text)
        self.assertIn("m_hubController->completeWorkspaceTransition();", route_bootstrap_impl_text)
        self.assertIn('QStringLiteral("navigationFailed")', route_bootstrap_impl_text)
        self.assertIn("The selected WhatSon Hub provider does not expose a mountable local directory path.", controller_text)
        self.assertIn("void OnboardingHubController::clearHubSelectionCandidates()", controller_text)
        self.assertIn("startAccessForUrl(hubUrl, true, &accessError)", controller_text)
        self.assertIn("ensureAccessForPath(normalizedCreatedHubPath, &accessError)", controller_text)
        self.assertIn("startAccessForUrl(hubUrl, false, &accessError)", controller_text)
        self.assertIn("ensureAccessForPath(mountableHubPath, &accessError)", controller_text)
        self.assertIn("return createHubAtUrl(WhatSon::HubPath::urlFromPath(targetHubPath));", controller_text)
        self.assertIn('const QString rawUrlText = hubUrl.toString(QUrl::FullyEncoded).trimmed();', controller_text)
        self.assertIn("if (WhatSon::Android::Storage::isSupportedUri(rawUrlText))", controller_text)
        self.assertIn('if (rawUrlText.startsWith(QStringLiteral("content:"), Qt::CaseInsensitive))', controller_text)
        self.assertIn("return WhatSon::HubPath::normalizePath(rawUrlText);", controller_text)
        self.assertIn("return rawUrlText;", controller_text)
        self.assertIn("return WhatSon::HubPath::pathFromUrl(hubUrl);", controller_text)
        self.assertIn("androidDocumentIdFromUrl(const QUrl& url)", hub_path_utils_text)
        self.assertIn("androidDownloadsPathFromDocumentId(const QString& documentId)", hub_path_utils_text)
        self.assertIn("resolveAndroidDocumentPath(const QUrl& url)", hub_path_utils_text)
        self.assertIn('authority == QStringLiteral("com.android.externalstorage.documents")', hub_path_utils_text)
        self.assertIn('authority == QStringLiteral("com.android.providers.downloads.documents")', hub_path_utils_text)
        self.assertIn('return {};', hub_path_utils_text)
        self.assertIn('trimmedDocumentId.startsWith(QStringLiteral("raw:"), Qt::CaseInsensitive)', hub_path_utils_text)
        self.assertIn("class Bridge", android_storage_header_text)
        self.assertIn("bool resolveHubSelection(", android_storage_header_text)
        self.assertIn("bool mountHub(", android_storage_header_text)
        self.assertIn("bool exportLocalHubToDirectory(", android_storage_header_text)
        self.assertIn("bool isMountedHubPath(const QString& path);", android_storage_header_text)
        self.assertIn("QString mountedHubSourceUri(const QString& mountedHubPath);", android_storage_header_text)
        self.assertIn('constexpr auto kMountsDirectoryName = "android-document-mounts";', android_storage_impl_text)
        self.assertIn("mountContainerPathForSource(const QString& sourceUri)", android_storage_impl_text)
        self.assertIn("uniqueDirectoryNameForParent(", android_storage_impl_text)
        self.assertIn("bool exportLocalHubToDirectory(", android_storage_impl_text)
        self.assertIn("bool isMountedHubPath(const QString& path)", android_storage_impl_text)
        self.assertIn("QString mountedHubSourceUri(const QString& mountedHubPath)", android_storage_impl_text)
        self.assertIn('trimmedPath.startsWith(QStringLiteral("content:"), Qt::CaseInsensitive)', android_storage_impl_text)
        self.assertIn('normalizedPath.startsWith(QStringLiteral("content:"), Qt::CaseInsensitive)', android_storage_impl_text)
        self.assertIn('url.scheme().compare(QStringLiteral("content"), Qt::CaseInsensitive) == 0', android_storage_impl_text)
        self.assertIn("implementation 'androidx.documentfile:documentfile:1.0.1'", android_build_gradle_text)
        self.assertIn("if (WhatSon::HubPath::isNonLocalUrl(normalizedSelectedPath))", controller_text)
        self.assertIn("const QString resolvedHubPath = resolveExistingHubPath(normalizedSelectedPath, &errorMessage);",
                      controller_text)
        self.assertIn("bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);",
                      ios_access_header_text)
        self.assertIn("bool ensureAccessForPath(const QString& localPath, QString* errorMessage);",
                      ios_access_header_text)
        self.assertIn("[nsUrl startAccessingSecurityScopedResource]", ios_access_impl_text)
        self.assertIn('QStringLiteral("ios.securityScoped")', ios_access_impl_text)
        self.assertIn("The selected iOS document URL is not a local filesystem URL.", ios_access_impl_text)
        self.assertIn("iOS denied access to the selected document provider URL.", ios_access_impl_text)
        self.assertNotIn("bookmarkDataForPath(", ios_access_impl_text)
        self.assertNotIn("restoreAccessFromBookmark(", ios_access_impl_text)
        self.assertIn('#include "platform/Android/WhatSonAndroidStorageBackend.hpp"', selected_hub_store_text)
        self.assertIn("const QString normalizedHubPath = WhatSon::HubPath::normalizePath(hubPath);", selected_hub_store_text)
        self.assertIn("WhatSon::Android::Storage::isSupportedUri(normalizedHubPath)", selected_hub_store_text)
        self.assertIn("WhatSon::Android::Storage::isMountedHubPath(normalizedHubPath)", selected_hub_store_text)
        self.assertIn("WhatSon::Android::Storage::mountedHubSourceUri(normalizedHubPath)", selected_hub_store_text)
        self.assertIn("return hubInfo.fileName().endsWith(QStringLiteral(\".wshub\"), Qt::CaseInsensitive);", selected_hub_store_text)
        self.assertNotIn('#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"', selected_hub_store_text)
        self.assertNotIn('workspace/selectedHubBookmark', selected_hub_store_text)
        self.assertIn("width: 180", onboarding_content_text)
        self.assertIn("width: parent.width", onboarding_content_text)
        self.assertIn("clearLastError();", onboarding_content_text)
        self.assertIn("readonly property string statusText:", onboarding_content_text)
        self.assertIn('root.onboardingSessionState === "routingWorkspace"', onboarding_content_text)
        self.assertIn('return "Opening WhatSon workspace...";', onboarding_content_text)
        self.assertIn('root.onboardingSessionState === "loadingHub"', onboarding_content_text)
        self.assertIn('return "Loading WhatSon Hub...";', onboarding_content_text)
        self.assertIn('root.onboardingSessionState === "resolvingSelection"', onboarding_content_text)
        self.assertIn('return "Resolving WhatSon Hub...";', onboarding_content_text)
        self.assertIn('readonly property string selectedHubStatusText:', onboarding_content_text)
        self.assertIn('return "No WhatSon Hub Selected";', onboarding_content_text)
        self.assertIn("target: root.hubSessionController", onboarding_content_text)
        self.assertIn("if (!root.standaloneMode && root.autoCompleteOnHubLoaded)", onboarding_content_text)
        self.assertIn("visible: root.useMobileLayout", onboarding_content_text)
        self.assertIn("width: root.mobileContentWidth", onboarding_content_text)
        self.assertIn("width: root.mobileActionWidth", onboarding_content_text)
        self.assertIn("color: LV.Theme.titleHeaderColor", onboarding_content_text)
        self.assertIn("lineHeight: 26", onboarding_content_text)
        self.assertIn("wrapMode: Text.NoWrap", onboarding_content_text)
        self.assertNotIn("mobileStatusPanelHeight", onboarding_content_text)
        self.assertNotIn("mobileTopContentWidth", onboarding_content_text)
        self.assertNotIn("id: mobileHubPanel", onboarding_content_text)
        self.assertNotIn("id: mobileAppPanel", onboarding_content_text)
        self.assertIn("property var hubSessionController: null", onboarding_content_text)
        self.assertIn("root.hubSessionController.currentHubPathName", onboarding_content_text)
        self.assertIn("root.hubSessionController.currentHubName", onboarding_content_text)
        self.assertIn(
            "Q_PROPERTY(QString currentHubName READ currentHubName NOTIFY currentHubNameChanged)",
            controller_header_text,
        )
        self.assertIn(
            "Q_PROPERTY(QString currentHubPathName READ currentHubPathName NOTIFY currentHubPathNameChanged)",
            controller_header_text,
        )
        self.assertIn("void syncCurrentHubSelection(const QString& hubPath);", controller_header_text)
        self.assertIn("bool createHubAtPath(", creator_header_text)


if __name__ == "__main__":
    unittest.main()
