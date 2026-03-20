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
        self.assertIn("readonly property int mobileDesignHeight: 760", onboarding_qml_text)
        self.assertIn("readonly property int mobileDesignWidth: 470", onboarding_qml_text)
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
        self.assertIn("radius: root.useMobileLayout ? 0 : 32", onboarding_qml_text)
        self.assertIn("radius: windowFrame.radius", onboarding_qml_text)
        self.assertIn("anchors.rightMargin: windowFrame.radius", onboarding_qml_text)
        self.assertIn("import QtQuick.Dialogs", onboarding_qml_text)
        self.assertIn("FileDialog {", onboarding_qml_text)
        self.assertIn('readonly property string defaultCreateHubFileName: "Untitled.wshub"', onboarding_qml_text)
        self.assertIn("readonly property url suggestedCreateHubFileUrl:", onboarding_qml_text)
        self.assertIn("currentFile: root.suggestedCreateHubFileUrl", onboarding_qml_text)
        self.assertIn("selectedFile: root.suggestedCreateHubFileUrl", onboarding_qml_text)
        self.assertIn("FolderDialog {", onboarding_qml_text)
        self.assertIn("createHubAtUrl(selectedFile)", onboarding_qml_text)
        self.assertIn("loadHubFromUrl(selectedFolder)", onboarding_qml_text)
        self.assertIn("width: 180", onboarding_qml_text)
        self.assertIn("width: parent.width", onboarding_qml_text)
        self.assertIn("clearLastError();", onboarding_qml_text)
        self.assertIn("readonly property string statusText:", onboarding_qml_text)
        self.assertIn('readonly property string selectedHubStatusText:', onboarding_qml_text)
        self.assertIn('return "No WhatSon Hub Selected";', onboarding_qml_text)
        self.assertIn("target: root.hubSessionController", onboarding_qml_text)
        self.assertIn("visible: root.useMobileLayout", onboarding_qml_text)
        self.assertIn("id: mobileHubPanel", onboarding_qml_text)
        self.assertIn("id: mobileAppPanel", onboarding_qml_text)
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
