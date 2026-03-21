from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class HubWriteLeaseTests(unittest.TestCase):
    def test_runtime_and_docs_reference_shared_hub_access(self) -> None:
        lease_header = (REPO_ROOT / "src/app/file/hub/WhatSonHubWriteLease.hpp").read_text(encoding="utf-8")
        app_main = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        io_gateway = (REPO_ROOT / "src/app/file/IO/WhatSonSystemIoGateway.cpp").read_text(encoding="utf-8")
        validator = (
            REPO_ROOT / "src/app/file/validator/WhatSonLibraryIndexIntegrityValidator.cpp"
        ).read_text(encoding="utf-8")
        note_deletion = (
            REPO_ROOT / "src/app/file/note/WhatSonHubNoteDeletionService.cpp"
        ).read_text(encoding="utf-8")
        readme = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn('QStringLiteral(".whatson/write-lease.json")', lease_header)
        self.assertIn("heartbeatIntervalMs()", lease_header)
        self.assertIn("timeoutMs()", lease_header)
        self.assertIn("refreshWriteLeaseForHub", lease_header)
        self.assertIn("releaseWriteLeaseForHub", lease_header)

        self.assertIn('#include "file/hub/WhatSonHubWriteLease.hpp"', app_main)
        self.assertIn("QTimer hubWriteLeaseTimer;", app_main)
        self.assertIn("hubWriteLeaseTimer.setInterval(WhatSon::HubWriteLease::heartbeatIntervalMs());", app_main)
        self.assertIn("updateWriteLeaseOwnership", app_main)
        self.assertIn("refreshWriteLeaseForHub(normalizedHubPath, &leaseError)", app_main)
        self.assertIn("releaseWriteLeaseForHub(currentWriteLeaseHubPath, nullptr);", app_main)

        self.assertIn("ensureWriteLeaseForPath(normalizedPath, &leaseError)", io_gateway)
        self.assertIn("ensureWriteLeaseForPath(filePath, &leaseError)", validator)
        self.assertIn("ensureWriteLeaseForPath(normalizedWshubPath, &leaseError)", note_deletion)

        self.assertIn(".whatson/write-lease.json", readme)
        self.assertIn("Concurrent hub access", readme)
        self.assertIn(".whatson/write-lease.json", architecture)
        self.assertIn("legacy cleanup shim", architecture)


if __name__ == "__main__":
    unittest.main()
