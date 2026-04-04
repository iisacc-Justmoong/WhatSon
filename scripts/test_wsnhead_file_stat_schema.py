#!/usr/bin/env python3

from __future__ import annotations

import sys
import xml.etree.ElementTree as ET
from pathlib import Path


REQUIRED_TAGS = [
    "totalFolders",
    "totalTags",
    "letterCount",
    "wordCount",
    "sentenceCount",
    "paragraphCount",
    "spaceCount",
    "indentCount",
    "lineCount",
    "openCount",
    "modifiedCount",
    "backlinkToCount",
    "backlinkByCount",
    "includedResourceCount",
]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def parse_header_file(path: Path) -> ET.Element:
    return ET.fromstring(path.read_text(encoding="utf-8"))


def assert_file_stat_block(path: Path) -> None:
    root = parse_header_file(path)
    file_stat = root.find("./head/fileStat")
    require(file_stat is not None, f"{path} is missing ./head/fileStat")
    for tag_name in REQUIRED_TAGS:
        tag = file_stat.find(tag_name)
        require(tag is not None, f"{path} is missing <{tag_name}>")
        require((tag.text or "").strip().isdigit(), f"{path} <{tag_name}> is not an integer")


def assert_text_contains(path: Path, needles: list[str]) -> None:
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        require(needle in text, f"{path} is missing text: {needle}")


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]

    assert_file_stat_block(
        repo_root
        / "blueprint/TestHub.wshub/TestHub.wscontents/Library.wslibrary/"
        / "bt9xHvb6dZ8LQoTQ-0UJjTdU7Dm6S26dp.wsnote/bt9xHvb6dZ8LQoTQ-0UJjTdU7Dm6S26dp.wsnhead"
    )
    assert_text_contains(
        repo_root
        / "blueprint/TestHub.wshub/TestHub.wscontents/Preset.wspreset/"
        / "ExampleNote.wsnote/ExampleNote.wsnhead",
        ["<fileStat>"] + [f"<{tag_name}>" for tag_name in REQUIRED_TAGS],
    )

    assert_text_contains(
        repo_root / "src/app/file/note/WhatSonNoteHeaderCreator.cpp",
        ["<fileStat>"] + [f"<{tag_name}>" for tag_name in REQUIRED_TAGS],
    )
    assert_text_contains(
        repo_root / "src/app/file/note/WhatSonNoteHeaderParser.cpp",
        [f"QStringLiteral(\"{tag_name}\")" for tag_name in REQUIRED_TAGS],
    )
    assert_text_contains(
        repo_root / "src/app/file/note/WhatSonNoteHeaderStore.hpp",
        REQUIRED_TAGS,
    )
    assert_text_contains(
        repo_root / "src/app/file/validator/WhatSonNoteStorageValidator.cpp",
        ["<fileStat>"] + [f"<{tag_name}>" for tag_name in REQUIRED_TAGS],
    )
    assert_text_contains(
        repo_root / "src/app/viewmodel/detailPanel/DetailFileStatViewModel.hpp",
        REQUIRED_TAGS + ["summaryLines", "textMetricLines", "activityLines", "overviewItems", "textItems", "relationItems"],
    )
    assert_text_contains(
        repo_root / "src/app/viewmodel/detailPanel/DetailFileStatViewModel.cpp",
        [
            "Projects: %1",
            "Total folders: %1",
            "Folders: %1",
            "Total tags: %1",
            "Tags: %1",
            "Created at: %1",
            "Modified at: %1",
            "Letter: %1",
            "Word: %1",
            "Sentence: %1",
            "Paragraph: %1",
            "Space: %1",
            "Indent: %1",
            "Line: %1",
            "Open count: %1",
            "Modified count: %1",
            "Backlink to: %1",
            "Backlink by: %1",
            "Include resources: %1",
        ],
    )
    assert_text_contains(
        repo_root / "src/app/viewmodel/detailPanel/DetailPanelViewModel.hpp",
        ["#include \"DetailFileStatViewModel.hpp\"", "DetailFileStatViewModel m_fileStatViewModel;"],
    )
    assert_text_contains(
        repo_root / "src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp",
        ["m_fileStatViewModel(this)", "m_fileStatViewModel.applyHeader(header);", "m_fileStatViewModel.clearHeader();"],
    )
    assert_text_contains(
        repo_root / "src/app/qml/view/panels/detail/DetailContents.qml",
        ["property var fileStatViewModel: null", "DetailFileStatForm {", "fileStatViewModel: detailContents.resolvedActiveStateName === \"fileStat\" ? detailContents.fileStatViewModel : null"],
    )
    assert_text_contains(
        repo_root / "src/app/qml/view/panels/detail/DetailFileStatForm.qml",
        [
            "summaryLines",
            "textMetricLines",
            "activityLines",
            "style: description",
            "LV.Theme.descriptionColor",
            "SummaryStatsSection",
            "TextMetricStatsSection",
            "ActivityStatsSection",
        ],
    )
    assert_text_contains(
        repo_root / "src/app/qml/view/panels/detail/DetailPanel.qml",
        ["resolvedFileStatViewModel", "resolveFileStatViewModel()", "fileStatViewModel: detailPanel.resolvedFileStatViewModel"],
    )
    assert_text_contains(
        repo_root / "src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp",
        ["Qt::QueuedConnection", "reloadNoteMetadataForNoteId"],
    )

    print("wsnhead fileStat schema static checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"wsnhead fileStat schema check failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
