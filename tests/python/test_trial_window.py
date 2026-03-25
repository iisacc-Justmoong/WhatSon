from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class TrialWindowTests(unittest.TestCase):
    def test_trial_build_loads_dedicated_status_window(self) -> None:
        app_cmake_text = (REPO_ROOT / "src/app/CMakeLists.txt").read_text(encoding="utf-8")
        app_main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        trial_window_text = (REPO_ROOT / "src/app/qml/window/TrialStatus.qml").read_text(encoding="utf-8")
        main_doc_text = (REPO_ROOT / "docs/src/app/main.cpp.md").read_text(encoding="utf-8")
        app_cmake_doc_text = (REPO_ROOT / "docs/src/app/CMakeLists.txt.md").read_text(encoding="utf-8")
        window_readme_text = (REPO_ROOT / "docs/src/app/qml/window/README.md").read_text(encoding="utf-8")
        trial_window_doc_text = (REPO_ROOT / "docs/src/app/qml/window/TrialStatus.qml.md").read_text(encoding="utf-8")

        self.assertIn("if (WHATSON_IS_TRIAL_BUILD AND NOT ANDROID AND NOT (APPLE AND CMAKE_SYSTEM_NAME STREQUAL \"iOS\"))", app_cmake_text)
        self.assertIn("${CMAKE_SOURCE_DIR}/src/extension/trial/WhatSonTrialSecureStore.cpp", app_cmake_text)
        self.assertIn("${CMAKE_SOURCE_DIR}/src/extension/trial/WhatSonTrialInstallStore.cpp", app_cmake_text)
        self.assertIn("${CMAKE_SOURCE_DIR}/src/extension/trial/WhatSonTrialActivationPolicy.cpp", app_cmake_text)
        self.assertIn("${CMAKE_SOURCE_DIR}/src/extension/trial", app_cmake_text)
        self.assertIn("WHATSON_IS_TRIAL_BUILD=1", app_cmake_text)
        self.assertIn("if (WHATSON_IS_TRIAL_BUILD AND NOT CMAKE_SYSTEM_NAME STREQUAL \"iOS\")", app_cmake_text)
        self.assertIn("find_library(WHATSON_SECURITY_FRAMEWORK Security REQUIRED)", app_cmake_text)

        self.assertIn('#include "WhatSonTrialActivationPolicy.hpp"', app_main_text)
        self.assertIn("WhatSonRegisterManager trialRegisterManager;", app_main_text)
        self.assertIn("WhatSonTrialActivationPolicy trialActivationPolicy;", app_main_text)
        self.assertIn("trialActivationPolicy.setRegisterManager(&trialRegisterManager);", app_main_text)
        self.assertIn("&WhatSonRegisterManager::authenticatedChanged", app_main_text)
        self.assertIn("&WhatSonTrialActivationPolicy::refresh", app_main_text)
        self.assertIn("trialActivationPolicy.refresh();", app_main_text)
        self.assertIn("const auto loadTrialStatusWindow =", app_main_text)
        self.assertIn('QStringLiteral("TrialStatus"),', app_main_text)
        self.assertIn('QStringLiteral("trialActivationPolicy")', app_main_text)
        self.assertIn('QStringLiteral("hostWindow")', app_main_text)
        self.assertIn("QObject* trialWindow = loadTrialStatusWindow(", app_main_text)
        self.assertIn("Failed to resolve the trial status window root object.", app_main_text)
        self.assertGreaterEqual(app_main_text.count("activateWindowObject(trialWindow);"), 2)

        self.assertIn("title: \"WhatSon Trial\"", trial_window_text)
        self.assertIn("property var trialActivationPolicy: null", trial_window_text)
        self.assertIn("readonly property int remainingDays:", trial_window_text)
        self.assertIn("readonly property string headlineText:", trial_window_text)
        self.assertIn("readonly property string primaryMetricText:", trial_window_text)
        self.assertIn("readonly property string summaryText:", trial_window_text)
        self.assertIn("text: root.primaryMetricText", trial_window_text)
        self.assertIn("text: \"Trial details\"", trial_window_text)
        self.assertIn("text: \"Trial length: \" + root.trialLengthDays + \" days\"", trial_window_text)
        self.assertIn("text: \"Elapsed: \" + root.elapsedDays + \" days\"", trial_window_text)
        self.assertIn("text: \"Installed: \" + root.formattedDate(", trial_window_text)
        self.assertIn("text: \"Last active day: \" + root.formattedDate(", trial_window_text)

        self.assertIn("Desktop trial builds pull in the dedicated trial activation sources", app_cmake_doc_text)
        self.assertIn("trial activation policy and a dedicated desktop trial-status window", main_doc_text)
        self.assertIn("TrialStatus.qml", window_readme_text)
        self.assertIn("Role", trial_window_doc_text)
        self.assertIn("desktop trial window", trial_window_doc_text)


if __name__ == "__main__":
    unittest.main()
