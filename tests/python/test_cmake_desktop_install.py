from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class CMakeDesktopInstallTests(unittest.TestCase):
    def test_native_desktop_export_uses_cmake_install(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn("set(_whatson_native_desktop_install_tree FALSE)", root_cmake_text)
        self.assertIn('elseif (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")', root_cmake_text)
        self.assertIn("elseif (UNIX AND NOT APPLE)", root_cmake_text)
        self.assertIn("COMMAND ${_whatson_install_dist_command}", root_cmake_text)
        self.assertIn('COMMENT "Export self-contained host install tree to ${WHATSON_DIST_DIR}"', root_cmake_text)

    def test_native_desktop_app_install_uses_qt_deploy_script(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn("AND _whatson_native_desktop_install_tree", root_cmake_text)
        self.assertIn("qt_generate_deploy_qml_app_script(", root_cmake_text)
        self.assertIn('install(SCRIPT "${WHATSON_QT_DEPLOY_APP_SCRIPT}")', root_cmake_text)

    def test_qt_root_path_is_added_to_prefix_path_for_all_hosts(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn("if (QT_ROOT_PATH)", root_cmake_text)
        self.assertIn('list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT_PATH}")', root_cmake_text)

    def test_windows_daemon_install_deploys_runtime_dependencies(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn("set(_whatson_native_desktop_runtime_deploy FALSE)", root_cmake_text)
        self.assertIn('set(_whatson_daemon_qt_conf_arg "    GENERATE_QT_CONF\\n")', root_cmake_text)
        self.assertIn(r"EXECUTABLE \$<TARGET_FILE:WhatSon_daemon>", root_cmake_text)
        self.assertIn("NO_TRANSLATIONS", root_cmake_text)

    def test_readme_documents_macos_and_windows_install_layout(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")

        self.assertIn(
            "On native desktop host builds, `whatson_export_binaries` now stages a self-contained install tree under "
            "`build/dist`",
            readme_text,
        )
        self.assertIn("cmake -S . -B build -DQT_ROOT_PATH=/absolute/path/to/Qt/kit", readme_text)
        self.assertIn("- macOS: `build/dist/WhatSon.app`", readme_text)
        self.assertIn("- Windows: `build/dist/bin/WhatSon.exe`", readme_text)
        self.assertIn("Host desktop app builds now emit the runnable app artifact at the build-directory root", readme_text)
        self.assertIn("- macOS: `build/WhatSon.app`", readme_text)
        self.assertIn("- Windows: `build/WhatSon.exe`", readme_text)

    def test_desktop_runtime_output_uses_build_root(self) -> None:
        app_cmake_text = (REPO_ROOT / "src/app/CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn('set(_whatson_app_runtime_output_dir "$<1:${CMAKE_CURRENT_BINARY_DIR}/bin>")', app_cmake_text)
        self.assertIn('if (NOT ANDROID AND NOT (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS"))', app_cmake_text)
        self.assertIn('set(_whatson_app_runtime_output_dir "$<1:${CMAKE_BINARY_DIR}>")', app_cmake_text)

    def test_cli_and_mobile_scripts_use_build_root_artifacts(self) -> None:
        cli_text = (REPO_ROOT / "src/cli/src/main.rs").read_text(encoding="utf-8")
        runner_text = (REPO_ROOT / "scripts/build_platform_runner.py").read_text(encoding="utf-8")

        self.assertIn('    "build/WhatSon.app/Contents/MacOS/WhatSon",', cli_text)
        self.assertIn('    "build-trial/WhatSon.app/Contents/MacOS/WhatSon",', cli_text)
        self.assertIn('    "build/WhatSon.exe",', cli_text)
        self.assertIn('parser.add_argument("--host-build-dir", default=str(repo_root / "build"))', runner_text)
        self.assertIn('parser.add_argument("--trial-build-dir", default=str(repo_root / "build-trial"))', runner_text)
        self.assertIn('destination=self.ios_project_dir / "WhatSon.app",', runner_text)
        self.assertIn('destination=self.android_build_dir / "WhatSon.apk",', runner_text)

    def test_root_cmake_syncs_trial_build_tree_for_host_builds(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn('option(WHATSON_IS_TRIAL_BUILD "Mark this build tree as the dedicated trial packaging tree."', root_cmake_text)
        self.assertIn('option(WHATSON_ENABLE_TRIAL_BUILD_MIRROR', root_cmake_text)
        self.assertIn('set(WHATSON_TRIAL_BUILD_DIR "${CMAKE_SOURCE_DIR}/build-trial" CACHE PATH', root_cmake_text)
        self.assertIn('set(WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES "${_WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES_DEFAULT}" CACHE STRING', root_cmake_text)
        self.assertIn('add_custom_target(whatson_sync_trial_build', root_cmake_text)
        self.assertIn('add_dependencies(whatson_build_all whatson_sync_trial_build)', root_cmake_text)
        self.assertIn('add_dependencies(WhatSon whatson_sync_trial_build)', root_cmake_text)
        self.assertIn('add_custom_target(whatson_sync_trial_build_on_build ALL', root_cmake_text)
        self.assertIn('"-DCMAKE_OSX_ARCHITECTURES=${WHATSON_TRIAL_BUILD_OSX_ARCHITECTURES}"', root_cmake_text)

    def test_readme_documents_root_build_creates_build_trial(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")

        self.assertIn("The root host build now also prepares `build-trial` as the dedicated desktop trial-packaging tree.", readme_text)
        self.assertIn("plain `cmake --build build`, `cmake --build build --target WhatSon`, CLion's `WhatSon`", readme_text)
        self.assertIn("build, and `whatson_build_all` all configure `build-trial`", readme_text)
        self.assertIn("On macOS the nested trial tree pins `CMAKE_OSX_ARCHITECTURES` from the host/LVRS setup", readme_text)
        self.assertIn("The same `build-trial` mirror is also created by the root CMake build path", readme_text)


if __name__ == "__main__":
    unittest.main()
