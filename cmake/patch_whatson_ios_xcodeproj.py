#!/usr/bin/env python3

from __future__ import annotations

import re
import sys
from pathlib import Path


def fail(message: str) -> int:
    print(message, file=sys.stderr)
    return 1


def strip_qt_permission_link_inputs(pbxproj_text: str) -> str:
    patched_text = pbxproj_text
    removal_patterns = (
        re.compile(
            r'^\s*"\'-Wl,-u,_QDarwinMicrophonePermissionRequest\'",\n',
            re.MULTILINE,
        ),
        re.compile(
            r" ?'-Wl,-u,_QDarwinMicrophonePermissionRequest'",
        ),
        re.compile(
            r'^\s*"[^"\n]*/plugins/permissions/objects-(?:Debug|Release)/'
            r'QDarwin(?:Bluetooth|Calendar|Location|Microphone)PermissionPlugin_init/[^"\n]+",\n',
            re.MULTILINE,
        ),
        re.compile(
            r',\s*"[^"\n]*/plugins/permissions/objects-(?:Debug|Release)/'
            r'QDarwin(?:Bluetooth|Calendar|Location|Microphone)PermissionPlugin_init/[^"\n]+"',
        ),
        re.compile(
            r'^\s*"?(?:[^,"\n]*/)?plugins/permissions/libqdarwin'
            r'(?:bluetooth|calendar|location|microphone)permission(?:_debug)?\.a"?,\n',
            re.MULTILINE,
        ),
        re.compile(
            r',\s*"?(?:[^,"\n]*/)?plugins/permissions/libqdarwin'
            r'(?:bluetooth|calendar|location|microphone)permission(?:_debug)?\.a"?',
        ),
    )

    for pattern in removal_patterns:
        patched_text = pattern.sub("", patched_text)

    return patched_text


def main() -> int:
    strip_qt_permissions = False
    argv = sys.argv[1:]
    if "--strip-qt-permissions" in argv:
        strip_qt_permissions = True
        argv.remove("--strip-qt-permissions")

    if len(argv) != 1:
        return fail(
            "usage: patch_whatson_ios_xcodeproj.py <path-to-project.pbxproj> [--strip-qt-permissions]"
        )

    pbxproj_path = Path(argv[0])
    if not pbxproj_path.is_file():
        return fail(f"WhatSon iOS Xcode project patch expected {pbxproj_path} to exist.")

    pbxproj_text = pbxproj_path.read_text()

    icon_build_file_match = re.search(
        r"^\s*([A-F0-9]+) /\* (.+WhatSonIcons\.xcassets) \*/ = \{isa = PBXBuildFile;",
        pbxproj_text,
        re.MULTILINE,
    )
    if icon_build_file_match is None:
        return fail("WhatSon iOS Xcode project patch could not find the asset catalog build file entry.")

    icon_build_file_id = icon_build_file_match.group(1)
    icon_build_file_comment = icon_build_file_match.group(2)

    target_match = re.search(
        r"^\s*[A-F0-9]+ /\* WhatSon \*/ = \{\n"
        r"\s*isa = PBXNativeTarget;\n"
        r".*?\n"
        r"\s*buildPhases = \(\n"
        r"(?P<buildphases>.*?)"
        r"\n\s*\);\n"
        r".*?\n"
        r"\s*name = WhatSon;\n",
        pbxproj_text,
        re.MULTILINE | re.DOTALL,
    )
    if target_match is None:
        return fail("WhatSon iOS Xcode project patch could not resolve the WhatSon PBXNativeTarget block.")

    build_phases_text = target_match.group("buildphases")
    resources_phase_match = re.search(
        r"^\s*([A-F0-9]+) /\* Resources \*/,?$",
        build_phases_text,
        re.MULTILINE,
    )
    if resources_phase_match is None:
        return fail("WhatSon iOS Xcode project patch could not resolve the WhatSon PBXResourcesBuildPhase.")

    resources_phase_id = resources_phase_match.group(1)
    resources_phase_pattern = re.compile(
        rf"(^\s*{resources_phase_id} /\* Resources \*/ = \{{\n"
        rf"\s*isa = PBXResourcesBuildPhase;\n"
        rf"\s*buildActionMask = 2147483647;\n"
        rf"\s*files = \(\n)"
        rf"(?P<files>.*?)"
        rf"(\n\s*\);\n"
        rf"\s*runOnlyForDeploymentPostprocessing = 0;\n"
        rf"\s*\}};)",
        re.MULTILINE | re.DOTALL,
    )
    resources_phase_block_match = resources_phase_pattern.search(pbxproj_text)
    if resources_phase_block_match is None:
        return fail("WhatSon iOS Xcode project patch could not find the WhatSon Resources build phase block.")

    files_block_text = resources_phase_block_match.group("files")
    icon_file_line_pattern = re.compile(
        rf"^\s*{re.escape(icon_build_file_id)} /\* .+WhatSonIcons\.xcassets \*/,?$",
        re.MULTILINE,
    )
    patched_pbxproj_text = pbxproj_text
    icon_was_missing = icon_file_line_pattern.search(files_block_text) is None
    if icon_was_missing:
        patched_files_block_text = (
            f"{files_block_text}\n"
            f"\t\t\t\t{icon_build_file_id} /* {icon_build_file_comment} */,"
        )
        patched_pbxproj_text = resources_phase_pattern.sub(
            rf"\1{patched_files_block_text}\3",
            patched_pbxproj_text,
            count=1,
        )

        patched_resources_phase_block_match = resources_phase_pattern.search(patched_pbxproj_text)
        if patched_resources_phase_block_match is None or icon_file_line_pattern.search(
            patched_resources_phase_block_match.group("files")
        ) is None:
            return fail(
                "WhatSon iOS Xcode project patch failed to attach WhatSonIcons.xcassets to the Resources build phase."
            )

    if strip_qt_permissions:
        patched_pbxproj_text = strip_qt_permission_link_inputs(patched_pbxproj_text)

    if patched_pbxproj_text == pbxproj_text:
        return 0

    pbxproj_path.write_text(patched_pbxproj_text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
