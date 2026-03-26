from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class StartupLazyLoadingTests(unittest.TestCase):
    def test_runtime_loader_supports_requested_domain_mask(self) -> None:
        loader_header_text = (
            REPO_ROOT / "src/app/runtime/threading/WhatSonRuntimeParallelLoader.hpp"
        ).read_text(encoding="utf-8")
        loader_impl_text = (
            REPO_ROOT / "src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("struct RequestedDomains", loader_header_text)
        self.assertIn("bool tags = true;", loader_header_text)
        self.assertIn("bool hubRuntimeStore = true;", loader_header_text)
        self.assertIn(
            "const RequestedDomains& requestedDomains = RequestedDomains{}",
            loader_header_text,
        )

        self.assertIn("if (requestedDomains.library)", loader_impl_text)
        self.assertIn("if (requestedDomains.tags)", loader_impl_text)
        self.assertIn("if (requestedDomains.hubRuntimeStore)", loader_impl_text)

    def test_startup_bootstrap_defers_secondary_hierarchy_domains(self) -> None:
        main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        app_readme_text = (REPO_ROOT / "docs/src/app/README.md").read_text(encoding="utf-8")

        self.assertIn("const auto loadStartupHubIntoRuntime =", main_text)
        self.assertIn("requestedDomains.event = false;", main_text)
        self.assertIn("requestedDomains.preset = false;", main_text)
        self.assertIn(
            "initialHubLoaded = loadStartupHubIntoRuntime(startupHubPath, &errorMessage);",
            main_text,
        )
        self.assertIn("startupDeferredBootstrapActive = true;", main_text)
        self.assertIn("ensureDeferredStartupHierarchyLoaded", main_text)
        self.assertIn("startup-idle-prefetch", main_text)
        self.assertIn("sidebar-activation", main_text)
        self.assertIn("Deferred startup load failed for domain", main_text)
        self.assertIn(
            "&SidebarHierarchyViewModel::activeHierarchyIndexChanged",
            main_text,
        )
        self.assertIn(
            "hubSyncController.setReloadCallback(loadHubIntoRuntime);",
            main_text,
        )

        self.assertIn("defers low-priority hierarchy domains", readme_text)
        self.assertIn("Defer low-priority hierarchy domains", app_readme_text)


if __name__ == "__main__":
    unittest.main()
