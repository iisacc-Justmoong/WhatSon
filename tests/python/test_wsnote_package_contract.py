from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class WsnotePackageContractTests(unittest.TestCase):
    def test_local_note_file_store_must_keep_four_file_contract(self) -> None:
        source_text = (
            REPO_ROOT / "src/app/file/note/WhatSonLocalNoteFileStore.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(".wsnhead", source_text)
        self.assertIn(".wsnbody", source_text)
        self.assertIn(".wsnversion", source_text)
        self.assertIn(".wsnpaint", source_text)
        self.assertNotIn(".wsnhistory", source_text)
        self.assertNotIn("QStringLiteral(\".meta\")", source_text)

    def test_note_creation_flow_must_not_generate_attachment_or_link_sidecars(self) -> None:
        creation_service_text = (
            REPO_ROOT / "src/app/file/note/WhatSonHubNoteCreationService.cpp"
        ).read_text(encoding="utf-8")

        self.assertNotIn("WhatSonNoteAttachManagerCreator", creation_service_text)
        self.assertNotIn("WhatSonNoteLinkManagerCreator", creation_service_text)
        self.assertNotIn("attachments.wsnpaint", creation_service_text)
        self.assertNotIn("links.wsnlink", creation_service_text)

    def test_attachment_and_link_creator_sources_must_be_removed(self) -> None:
        self.assertFalse(
            (REPO_ROOT / "src/app/file/note/WhatSonNoteAttachManagerCreator.hpp").exists()
        )
        self.assertFalse(
            (REPO_ROOT / "src/app/file/note/WhatSonNoteAttachManagerCreator.cpp").exists()
        )
        self.assertFalse(
            (REPO_ROOT / "src/app/file/note/WhatSonNoteLinkManagerCreator.hpp").exists()
        )
        self.assertFalse(
            (REPO_ROOT / "src/app/file/note/WhatSonNoteLinkManagerCreator.cpp").exists()
        )

    def test_local_note_store_doc_must_describe_paint_contract(self) -> None:
        doc_text = (
            REPO_ROOT / "docs/src/app/file/note/WhatSonLocalNoteFileStore.cpp.md"
        ).read_text(encoding="utf-8")

        self.assertIn(".wsnpaint", doc_text)
        self.assertIn(".wsnversion", doc_text)
        self.assertIn("no longer creates or appends `.wsnhistory`", doc_text)


if __name__ == "__main__":
    unittest.main()
