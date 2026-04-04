#!/usr/bin/env python3

from __future__ import annotations

import sys
from pathlib import Path


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def assert_text_contains(path: Path, needles: list[str]) -> None:
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        require(needle in text, f"{path} is missing text: {needle}")


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    controller_path = repo_root / "src/app/qml/view/panels/sidebar/SidebarHierarchyNoteDropController.qml"
    controller_doc_path = repo_root / "docs/src/app/qml/view/panels/sidebar/SidebarHierarchyNoteDropController.qml.md"
    view_doc_path = repo_root / "docs/src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml.md"

    assert_text_contains(
        controller_path,
        [
            "function collectHierarchyItems() {",
            "return items;",
            "function normalizeNoteIds(noteIds) {",
            "return normalized;",
            "const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);",
            "const draggedNoteIds = noteDropController.normalizeNoteIds(source.draggedNoteIds);",
        ],
    )

    assert_text_contains(
        controller_doc_path,
        [
            "scripts/test_sidebar_note_drop_controller.py",
            "return normalized;",
            "return items;",
        ],
    )

    assert_text_contains(
        view_doc_path,
        [
            "return normalized;",
            "always-empty payload",
        ],
    )

    print("sidebar note drop controller static checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"sidebar note drop controller check failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
