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
    listbar_path = repo_root / "src/app/qml/view/panels/ListBarLayout.qml"

    assert_text_contains(
        listbar_path,
        [
            "property bool displayedNoteListEntriesSyncDeferred: false",
            "property bool displayedNoteListEntriesForceRefreshDeferred: false",
            "function flushDeferredDisplayedNoteListEntriesSync()",
            "function requestDisplayedNoteListEntriesSync(forceRefresh)",
            "if (listBarLayout.noteDragActive) {",
            "displayedNoteListEntriesSyncDeferred = true;",
            "displayedNoteListEntriesForceRefreshDeferred = listBarLayout.displayedNoteListEntriesForceRefreshDeferred || requestedForceRefresh;",
            "onNoteDragActiveChanged:",
            "listBarLayout.flushDeferredDisplayedNoteListEntriesSync();",
            "reuseItems: !listBarLayout.noteDragActive",
            "requestDisplayedNoteListEntriesSync(false);",
            "function selectedNoteIdsFromIndices(indices) {",
            "return noteIds;",
            "function uniqueTrimmedStringList(values) {",
            "return normalized;",
            "function deleteNoteIds(noteIds) {",
            "return deletedAny;",
            "function clearFoldersForNoteIds(noteIds) {",
            "return clearedAny;",
        ],
    )

    print("list bar drag stability static checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"list bar drag stability check failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
