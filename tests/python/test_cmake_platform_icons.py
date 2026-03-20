from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class CMakePlatformIconTests(unittest.TestCase):
    def test_app_target_declares_platform_icon_assets(self) -> None:
        cmake_text = (REPO_ROOT / "src/app/CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn('set(WHATSON_APP_ICON_ICNS "${WHATSON_APP_ICON_DESKTOP_DIR}/AppIcon.icns")', cmake_text)
        self.assertIn('set(WHATSON_APP_ICON_ICO "${WHATSON_APP_ICON_DESKTOP_DIR}/AppIcon.ico")', cmake_text)
        self.assertIn('XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"', cmake_text)
        self.assertIn('MACOSX_PACKAGE_LOCATION "Resources"', cmake_text)
        self.assertIn('XCODE_EXPLICIT_FILE_TYPE "folder.assetcatalog"', cmake_text)
        self.assertIn('${WHATSON_APP_ICON_ANDROID_DIR}/${_whatson_android_density}/AppIcon.png', cmake_text)
        self.assertIn('set(_whatson_android_default_icon_dst_dir "${_whatson_android_package_source_dir}/res/drawable")',
                      cmake_text)
        self.assertIn('"${_whatson_android_default_icon_dst_dir}/app_icon.png"', cmake_text)
        self.assertIn('ANDROID_PACKAGE_SOURCE_DIR "${_whatson_android_package_source_dir}"', cmake_text)
        self.assertIn('MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"', cmake_text)

    def test_linux_install_keeps_desktop_and_pixmap_icons(self) -> None:
        root_cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn('DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps"', root_cmake_text)
        self.assertIn('DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps"', root_cmake_text)
        self.assertIn('RENAME "whatson.png"', root_cmake_text)

    def test_repository_organizes_icon_assets_by_category(self) -> None:
        desktop_dir = REPO_ROOT / "resources/icons/app/desktop"
        ios_dir = REPO_ROOT / "resources/icons/app/ios"
        android_dir = REPO_ROOT / "resources/icons/app/android"
        watchos_dir = REPO_ROOT / "resources/icons/app/watchos"
        store_dir = REPO_ROOT / "resources/icons/app/store/googleplay"
        source_dir = REPO_ROOT / "resources/icons/app/source"
        onboarding_dir = REPO_ROOT / "resources/illustrations/onboarding"

        self.assertTrue((desktop_dir / "AppIcon.icns").is_file())
        self.assertTrue((desktop_dir / "AppIcon.ico").is_file())
        self.assertTrue((desktop_dir / "AppIcon.png").is_file())
        self.assertTrue((ios_dir / "AppIcon@ios~marketing.png").is_file())
        self.assertTrue((android_dir / "xxxhdpi/AppIcon.png").is_file())
        self.assertTrue((watchos_dir / "AppIcon~watch-marketing.png").is_file())
        self.assertTrue((store_dir / "AppIcon.png").is_file())
        self.assertTrue((source_dir / "AppIcon.af").is_file())
        self.assertTrue((source_dir / "AppIcon.icon/icon.json").is_file())
        self.assertTrue((onboarding_dir / "Onboarding.png").is_file())
        self.assertTrue((onboarding_dir / "Onboarding.af").is_file())

        self.assertFalse((REPO_ROOT / "resources/AppIcon.icns").exists())
        self.assertFalse((REPO_ROOT / "resources/AppIcon.ico").exists())
        self.assertFalse((REPO_ROOT / "resources/AppIcon.png").exists())
        self.assertFalse((REPO_ROOT / "resources/AppIconios~marketing.png").exists())
        self.assertFalse((REPO_ROOT / "resources/xxxhdpi/AppIcon.png").exists())
        self.assertFalse((REPO_ROOT / "resources/googleplay/AppIcon.png").exists())
        self.assertFalse((REPO_ROOT / "resources/Onboarding.png").exists())

    def test_macos_info_plist_declares_bundle_icon_file(self) -> None:
        plist_text = (REPO_ROOT / "platform/Apple/Info.plist").read_text(encoding="utf-8")

        self.assertIn("<key>CFBundleIconFile</key>", plist_text)
        self.assertIn("<string>AppIcon.icns</string>", plist_text)

    def test_ios_info_plist_uses_asset_catalog_icons(self) -> None:
        plist_text = (REPO_ROOT / "platform/Apple/iOS/Info.plist").read_text(encoding="utf-8")

        self.assertNotIn("<key>CFBundleIconFile</key>", plist_text)
        self.assertNotIn("<string>AppIcon.png</string>", plist_text)
        self.assertIn("<key>UIRequiresFullScreen</key>", plist_text)
        self.assertIn("<true/>", plist_text)
        self.assertIn("<key>UILaunchStoryboardName</key>", plist_text)
        self.assertIn("<string>LaunchScreen</string>", plist_text)

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
