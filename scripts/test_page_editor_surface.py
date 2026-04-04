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
    inline_editor = repo_root / "src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"
    desktop_editor = repo_root / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
    mobile_editor = repo_root / "src/app/qml/view/content/editor/MobileContentsDisplayView.qml"
    inline_doc = repo_root / "docs/src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml.md"
    desktop_doc = repo_root / "docs/src/app/qml/view/content/editor/ContentsDisplayView.qml.md"
    mobile_doc = repo_root / "docs/src/app/qml/view/content/editor/MobileContentsDisplayView.qml.md"
    readme_doc = repo_root / "docs/src/app/qml/view/content/editor/README.md"

    assert_text_contains(
        inline_editor,
        [
            "property bool externalScroll: false",
            "property var externalScrollViewport: null",
            "readonly property real contentOffsetY:",
            "readonly property var resolvedFlickable:",
            "interactive: !control.externalScroll && contentHeight > height",
        ],
    )

    for editor_path in [desktop_editor, mobile_editor]:
        assert_text_contains(
            editor_path,
            [
                "readonly property int printDocumentPageCount:",
                "readonly property real printPaperDocumentHeight:",
                "id: printDocumentViewport",
                "id: printDocumentSurface",
                "id: printPaperColumn",
                "externalScroll: contentsView.showPrintEditorLayout",
                "externalScrollViewport: contentsView.showPrintEditorLayout ? printDocumentViewport : null",
                "parent: contentsView.showPrintEditorLayout ? printDocumentSurface : editorViewport",
                "return contentEditor.resolvedFlickable;",
            ],
        )

    assert_text_contains(
        inline_doc,
        [
            "externalScroll: true",
            "externalScrollViewport",
            "outer paper document",
        ],
    )
    assert_text_contains(
        desktop_doc,
        [
            "printDocumentViewport",
            "printDocumentPageCount",
            "non-functional scaffold",
        ],
    )
    assert_text_contains(
        mobile_doc,
        [
            "external paper-document scroll contract",
            "printDocumentPageCount",
            "nested fixed-height editor frame",
        ],
    )
    assert_text_contains(
        readme_doc,
        [
            "outer paper-document viewport",
            "fixed-height scaffold",
        ],
    )

    print("page editor surface static checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"page editor surface check failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
