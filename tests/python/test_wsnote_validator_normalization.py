from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class WsnoteValidatorNormalizationTests(unittest.TestCase):
    def test_note_storage_validator_exposes_normalization_api(self) -> None:
        header_text = (
            REPO_ROOT / "src/app/file/validator/WhatSonNoteStorageValidator.hpp"
        ).read_text(encoding="utf-8")

        self.assertIn("normalizeWsnotePackage", header_text)
        self.assertIn("normalizeWsnotePackageByDirectoryPath", header_text)

    def test_note_storage_validator_normalizes_to_four_file_contract(self) -> None:
        source_text = (
            REPO_ROOT / "src/app/file/validator/WhatSonNoteStorageValidator.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(".wsnhead", source_text)
        self.assertIn(".wsnbody", source_text)
        self.assertIn(".wsnversion", source_text)
        self.assertIn(".wsnpaint", source_text)
        self.assertIn("allowedFileNames", source_text)
        self.assertIn("QDir::Hidden", source_text)
        self.assertIn("removeDirectoryRecursively", source_text)
        self.assertIn("removeFile", source_text)

    def test_library_integrity_validator_normalizes_before_pruning(self) -> None:
        source_text = (
            REPO_ROOT
            / "src/app/file/validator/WhatSonLibraryIndexIntegrityValidator.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("normalizeWsnotePackage(record, &normalizationError)", source_text)
        self.assertIn("notePackage.normalizeFailed", source_text)


if __name__ == "__main__":
    unittest.main()
