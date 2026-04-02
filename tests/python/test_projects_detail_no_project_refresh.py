from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class ProjectsDetailNoProjectRefreshContractTests(unittest.TestCase):
    def test_projects_vm_must_force_projection_reindex_when_project_membership_changes(self) -> None:
        source_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("const QString previousProjectLabel = indexedNote.project.trimmed();", source_text)
        self.assertIn("projectMembershipChanged", source_text)
        self.assertIn("refreshIndexedNotesFromProjectsFilePath(&refreshError)", source_text)
        self.assertIn(
            'QStringLiteral("reloadNoteMetadataForNoteId.projectionRefreshFailed")',
            source_text,
        )

    def test_detail_panel_cpp_test_must_cover_untitled_to_no_project_drop(self) -> None:
        cpp_test_text = (
            REPO_ROOT / "tests/app/test_detail_panel_viewmodel.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "detailWrites_mustDropSelectedProjectsNoteImmediatelyWhenProjectCleared",
            cpp_test_text,
        )
        self.assertIn(
            'QCOMPARE(noteCountByLabel(QStringLiteral("Untitled")), 0);',
            cpp_test_text,
        )
        self.assertIn(
            "QCOMPARE(noteListModel->rowCount(), 0);",
            cpp_test_text,
        )

    def test_projects_doc_must_document_membership_change_full_reindex(self) -> None:
        doc_text = (
            REPO_ROOT / "docs/src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.cpp.md"
        ).read_text(encoding="utf-8")

        self.assertIn("project label actually changed", doc_text)
        self.assertIn("full re-index pass", doc_text)


if __name__ == "__main__":
    unittest.main()
