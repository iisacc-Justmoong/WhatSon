from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class CMakePlatformIconTests(unittest.TestCase):
    def test_app_target_declares_platform_icon_assets(self) -> None:
        cmake_text = (REPO_ROOT / "src/app/CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn('set(WHATSON_APP_ICON_ICNS "${CMAKE_SOURCE_DIR}/resources/AppIcon.icns")', cmake_text)
        self.assertIn('set(WHATSON_APP_ICON_ICO "${CMAKE_SOURCE_DIR}/resources/AppIcon.ico")', cmake_text)
        self.assertIn('XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"', cmake_text)
        self.assertIn('resources/${_whatson_android_density}/AppIcon.png', cmake_text)
        self.assertIn('ANDROID_PACKAGE_SOURCE_DIR "${_whatson_android_package_source_dir}"', cmake_text)
        self.assertIn('MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"', cmake_text)

    def test_linux_install_keeps_desktop_and_pixmap_icons(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn('DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps"', root_cmake_text)
        self.assertIn('DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps"', root_cmake_text)
        self.assertIn('RENAME "whatson.png"', root_cmake_text)

    def test_macos_info_plist_declares_bundle_icon_file(self) -> None:
        plist_text = (REPO_ROOT / "platform/Apple/Info.plist").read_text(encoding="utf-8")

        self.assertIn("<key>CFBundleIconFile</key>", plist_text)
        self.assertIn("<string>AppIcon.icns</string>", plist_text)

    def test_repository_uses_windows_safe_icon_filenames(self) -> None:
        invalid_chars = set('<>:"/\\\\|?*')
        reserved_names = {
            *(f"COM{i}" for i in range(1, 10)),
            *(f"LPT{i}" for i in range(1, 10)),
            "CON",
            "PRN",
            "AUX",
            "NUL",
        }
        skipped_roots = {"build", ".git", "__pycache__"}

        for path in REPO_ROOT.rglob("*"):
            if not path.exists():
                continue
            relative_parts = path.relative_to(REPO_ROOT).parts
            if any(part in skipped_roots for part in relative_parts):
                continue

            for part in relative_parts:
                self.assertEqual(part, part.rstrip(" ."), msg=f"Windows-unsafe trailing character in {path}")
                self.assertFalse(any(char in invalid_chars for char in part),
                                 msg=f"Windows-unsafe character in {path}")
                self.assertNotIn(part.split(".")[0].upper(), reserved_names,
                                 msg=f"Windows reserved filename in {path}")


if __name__ == "__main__":
    unittest.main()
