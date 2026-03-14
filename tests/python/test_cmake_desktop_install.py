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


if __name__ == "__main__":
    unittest.main()
