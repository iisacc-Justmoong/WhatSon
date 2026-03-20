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
        creator_header_text = (
                REPO_ROOT / "src/app/file/hub/WhatSonHubCreator.hpp"
        ).read_text(encoding="utf-8")
        controller_header_text = (
                REPO_ROOT / "src/app/viewmodel/onboarding/OnboardingHubController.hpp"
        ).read_text(encoding="utf-8")
        controller_text = (
                REPO_ROOT / "src/app/viewmodel/onboarding/OnboardingHubController.cpp"
        ).read_text(encoding="utf-8")
        hub_path_utils_text = (
                REPO_ROOT / "src/app/file/hub/WhatSonHubPathUtils.hpp"
        ).read_text(encoding="utf-8")
        ios_access_header_text = (
                REPO_ROOT / "src/app/platform/Apple/AppleSecurityScopedResourceAccess.hpp"
        ).read_text(encoding="utf-8")
        ios_access_impl_text = (
                REPO_ROOT / "src/app/platform/Apple/AppleSecurityScopedResourceAccess.mm"
        ).read_text(encoding="utf-8")

        self.assertIn("#include <QCommandLineParser>", app_main_text)
        self.assertIn('QStringLiteral("onboarding-only")', app_main_text)
        self.assertIn(
            'engine.loadFromModule(QStringLiteral("WhatSon.App"), QStringLiteral("Onboarding"));',
            app_main_text,
        )
        self.assertIn('engine.loadFromModule(QStringLiteral("WhatSon.App"), QStringLiteral("Main"));', app_main_text)
        self.assertIn("mobileStandaloneOnboarding", app_main_text)
        self.assertIn("if (launchOptions.onboardingOnly || mobileStandaloneOnboarding)", app_main_text)
        self.assertIn('onboardingWindow->setProperty("standaloneMode", true);', app_main_text)
        self.assertIn('mainWindow->setProperty("onboardingVisible", true);', app_main_text)
        self.assertIn('"hubSessionController"', app_main_text)
        self.assertIn('"onboardingHubController"', app_main_text)
        self.assertIn("onboardingHubController.syncCurrentHubSelection(startupHubPath);", app_main_text)
        self.assertIn("SelectedHubStore selectedHubStore;", app_main_text)
        self.assertIn("selectedHubStore.startupHubPath(resolveBlueprintHubPath())", app_main_text)
        self.assertIn("selectedHubStore.setSelectedHubPath(hubPath);", app_main_text)
        self.assertIn("#include <QWindow>", app_main_text)
        self.assertIn("const auto activateWindowObject = [](QObject* windowObject)", app_main_text)
        self.assertIn("if (auto* window = qobject_cast<QWindow*>(windowObject))", app_main_text)
        self.assertIn("window->show();", app_main_text)
        self.assertIn("window->requestActivate();", app_main_text)
        self.assertIn("const QMetaObject::Connection onboardingDismissedConnection = QObject::connect(", app_main_text)
        self.assertIn("SIGNAL(dismissed())", app_main_text)
        self.assertIn("SLOT(quit())", app_main_text)
        self.assertIn("&loadMainWindow", app_main_text)
        self.assertIn("workspaceMainWindow = loadMainWindow();", app_main_text)
        self.assertIn("QObject::disconnect(onboardingDismissedConnection);", app_main_text)
        self.assertIn("&activateWindowObject", app_main_text)
        self.assertIn("activateWindowObject(workspaceMainWindow);", app_main_text)
        self.assertIn("activateWindowObject(mainWindow);", app_main_text)
        self.assertIn("QTimer::singleShot(0, &app, [onboardingWindowGuard]()", app_main_text)
        self.assertNotIn("onboardingWindowGuard->deleteLater();", app_main_text)
        load_main_index = app_main_text.find("workspaceMainWindow = loadMainWindow();")
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
        self.assertIn("onOnboardingVisibleChanged:", main_qml_text)
        self.assertIn("property var onboardingHubController: null", main_qml_text)
        self.assertIn("hubSessionController: applicationWindow.onboardingHubController", main_qml_text)
        self.assertIn("readonly property int desktopMinHeight: 420", onboarding_qml_text)
        self.assertIn("readonly property int desktopMinWidth: 620", onboarding_qml_text)
        self.assertIn(
            "readonly property bool hasHubSelectionCandidates: root.hubSessionController && root.hubSessionController.hubSelectionCandidateNames.length > 0",
            onboarding_qml_text,
        )
        self.assertIn('readonly property bool useAndroidExistingHubFileFlow: Qt.platform.os === "android"', onboarding_qml_text)
        self.assertIn("readonly property bool useMobileCreateDirectoryFlow: root.isMobilePlatform", onboarding_qml_text)
        self.assertIn("readonly property int mobileActionSpacing: 24", onboarding_qml_text)
        self.assertIn("readonly property int mobileActionWidth: 180", onboarding_qml_text)
        self.assertIn("readonly property int mobileContentSpacing: 32", onboarding_qml_text)
        self.assertIn("readonly property int mobileContentWidth: 209", onboarding_qml_text)
        self.assertIn("readonly property int mobileDesignHeight: 762", onboarding_qml_text)
        self.assertIn("readonly property int mobileDesignWidth: 470", onboarding_qml_text)
        self.assertIn("readonly property int mobileSurfaceRadius: 32", onboarding_qml_text)
        self.assertIn("readonly property int mobileVersionWidth: 75", onboarding_qml_text)
        self.assertIn("readonly property bool useMobileLayout:", onboarding_qml_text)
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
        self.assertIn("readonly property int dragRegionHeight: 72", onboarding_qml_text)
        self.assertIn("typeof root.startSystemMove === \"function\"", onboarding_qml_text)
        self.assertIn("cursorShape: Qt.SizeAllCursor", onboarding_qml_text)
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
        self.assertIn("radius: root.useMobileLayout ? root.mobileSurfaceRadius : 32", onboarding_qml_text)
        self.assertIn("radius: windowFrame.radius", onboarding_qml_text)
        self.assertIn("anchors.rightMargin: windowFrame.radius", onboarding_qml_text)
        self.assertIn("import QtQuick.Dialogs", onboarding_qml_text)
        self.assertIn("FileDialog {", onboarding_qml_text)
        self.assertIn('readonly property string defaultCreateHubFileName: "Untitled.wshub"', onboarding_qml_text)
        self.assertIn("readonly property url suggestedCreateHubFileUrl:", onboarding_qml_text)
        self.assertIn("currentFile: root.suggestedCreateHubFileUrl", onboarding_qml_text)
        self.assertIn("selectedFile: root.suggestedCreateHubFileUrl", onboarding_qml_text)
        self.assertIn("id: createHubDirectoryDialog", onboarding_qml_text)
        self.assertIn('title: "Choose Folder for New WhatSon Hub"', onboarding_qml_text)
        self.assertIn("FolderDialog {", onboarding_qml_text)
        self.assertIn("createHubInDirectoryUrl(selectedFolder, root.defaultCreateHubFileName)", onboarding_qml_text)
        self.assertIn("if (root.useMobileCreateDirectoryFlow)", onboarding_qml_text)
        self.assertIn("createHubAtUrl(selectedFile)", onboarding_qml_text)
        self.assertIn("id: selectHubFileDialog", onboarding_qml_text)
        self.assertIn("fileMode: FileDialog.OpenFile", onboarding_qml_text)
        self.assertIn('nameFilters: ["WhatSon Hub (*.wshub)"]', onboarding_qml_text)
        self.assertIn("root.hubSessionController.loadHubFromUrl(selectedFile);", onboarding_qml_text)
        self.assertIn('title: root.isMobilePlatform ? "Choose Folder Containing WhatSon Hub" : "Select WhatSon Hub"',
                      onboarding_qml_text)
        self.assertIn("prepareHubSelectionFromUrl(selectedFolder)", onboarding_qml_text)
        self.assertIn("loadHubFromUrl(selectedFolder)", onboarding_qml_text)
        self.assertIn("if (root.useAndroidExistingHubFileFlow)", onboarding_qml_text)
        self.assertIn("selectHubFileDialog.open();", onboarding_qml_text)
        self.assertIn("loadHubSelectionCandidate(index)", onboarding_qml_text)
        self.assertIn("readonly property string mobileSelectionAssistText:", onboarding_qml_text)
        self.assertIn("On Android, choose the .wshub package directly from the native picker.", onboarding_qml_text)
        self.assertIn("Choose the WhatSon Hub package found in the selected folder.", onboarding_qml_text)
        self.assertIn("On mobile, choose the folder that contains your WhatSon Hub.", onboarding_qml_text)
        self.assertIn("root.hubSessionController.clearHubSelectionCandidates();", onboarding_qml_text)
        self.assertIn("root.hubSessionController.hubSelectionCandidateNames", onboarding_qml_text)
        self.assertIn("Q_INVOKABLE bool createHubInDirectoryUrl(const QUrl& directoryUrl, const QString& preferredFileName);",
                      controller_header_text)
        self.assertIn(
            "Q_PROPERTY(QStringList hubSelectionCandidateNames READ hubSelectionCandidateNames NOTIFY hubSelectionCandidatesChanged)",
            controller_header_text,
        )
        self.assertIn("Q_INVOKABLE bool prepareHubSelectionFromUrl(const QUrl& hubUrl);", controller_header_text)
        self.assertIn("Q_INVOKABLE bool loadHubSelectionCandidate(int index);", controller_header_text)
        self.assertIn("void clearHubSelectionCandidates();", controller_header_text)
        self.assertIn('#include "file/hub/WhatSonHubPathUtils.hpp"', controller_text)
        self.assertIn('#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"', controller_text)
        self.assertIn('constexpr auto kDefaultMobileHubFileName = "Untitled.wshub";', controller_text)
        self.assertIn("uniqueHubPackagePathInDirectory(directoryPath, preferredFileName)", controller_text)
        self.assertIn("WhatSon::HubPath::joinPath(normalizedDirectoryPath, candidateName)", controller_text)
        self.assertIn("candidateName = QStringLiteral(\"%1-%2%3\")", controller_text)
        self.assertIn("bool OnboardingHubController::prepareHubSelectionFromUrl(const QUrl& hubUrl)", controller_text)
        self.assertIn("const QStringList packageCandidates = hubPackageCandidatesInDirectory(normalizedSelectedPath);",
                      controller_text)
        self.assertIn("setHubSelectionCandidatePaths(packageCandidates);", controller_text)
        self.assertIn("bool OnboardingHubController::loadHubSelectionCandidate(int index)", controller_text)
        self.assertIn("bool OnboardingHubController::loadResolvedHubPath(", controller_text)
        self.assertIn("void OnboardingHubController::clearHubSelectionCandidates()", controller_text)
        self.assertIn("startAccessForUrl(hubUrl, true, &accessError)", controller_text)
        self.assertIn("ensureAccessForPath(normalizedCreatedHubPath, &accessError)", controller_text)
        self.assertIn("startAccessForUrl(hubUrl, false, &accessError)", controller_text)
        self.assertIn("ensureAccessForPath(resolvedHubPath, &accessError)", controller_text)
        self.assertIn("return createHubAtUrl(WhatSon::HubPath::urlFromPath(targetHubPath));", controller_text)
        self.assertIn("return WhatSon::HubPath::pathFromUrl(hubUrl);", controller_text)
        self.assertIn("androidDocumentIdFromUrl(const QUrl& url)", hub_path_utils_text)
        self.assertIn("resolveAndroidDocumentPath(const QUrl& url)", hub_path_utils_text)
        self.assertIn('authority == QStringLiteral("com.android.externalstorage.documents")', hub_path_utils_text)
        self.assertIn('QStringLiteral("/storage/emulated/0")', hub_path_utils_text)
        self.assertIn('documentId.startsWith(QStringLiteral("raw:"), Qt::CaseInsensitive)', hub_path_utils_text)
        self.assertIn("bool startAccessForUrl(const QUrl& url, bool parentDirectoryScope, QString* errorMessage);",
                      ios_access_header_text)
        self.assertIn("bool ensureAccessForPath(const QString& localPath, QString* errorMessage);",
                      ios_access_header_text)
        self.assertIn("[nsUrl startAccessingSecurityScopedResource]", ios_access_impl_text)
        self.assertIn("The selected iOS document URL is not a local filesystem URL.", ios_access_impl_text)
        self.assertIn("iOS denied access to the selected document provider URL.", ios_access_impl_text)
        self.assertIn("width: 180", onboarding_qml_text)
        self.assertIn("width: parent.width", onboarding_qml_text)
        self.assertIn("clearLastError();", onboarding_qml_text)
        self.assertIn("readonly property string statusText:", onboarding_qml_text)
        self.assertIn('readonly property string selectedHubStatusText:', onboarding_qml_text)
        self.assertIn('return "No WhatSon Hub Selected";', onboarding_qml_text)
        self.assertIn("target: root.hubSessionController", onboarding_qml_text)
        self.assertIn("visible: root.useMobileLayout", onboarding_qml_text)
        self.assertIn("width: root.mobileContentWidth", onboarding_qml_text)
        self.assertIn("width: root.mobileActionWidth", onboarding_qml_text)
        self.assertIn("color: LV.Theme.titleHeaderColor", onboarding_qml_text)
        self.assertIn("lineHeight: 26", onboarding_qml_text)
        self.assertIn("wrapMode: Text.NoWrap", onboarding_qml_text)
        self.assertNotIn("mobileStatusPanelHeight", onboarding_qml_text)
        self.assertNotIn("mobileTopContentWidth", onboarding_qml_text)
        self.assertNotIn("id: mobileHubPanel", onboarding_qml_text)
        self.assertNotIn("id: mobileAppPanel", onboarding_qml_text)
        self.assertIn("property var hubSessionController: null", onboarding_qml_text)
        self.assertIn("root.hubSessionController.currentHubPathName", onboarding_qml_text)
        self.assertIn("root.hubSessionController.currentHubName", onboarding_qml_text)
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
