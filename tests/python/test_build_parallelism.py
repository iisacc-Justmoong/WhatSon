from __future__ import annotations

import argparse
import errno
import io
import json
import plistlib
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

import build_all as build_all_script
import build_platform_runner as runner
import dev_env as dev_env_script
import runtime_smoke_matrix as smoke_matrix


class RecordingBuildAll(runner.BuildAll):
    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        self.commands: list[list[str]] = []

    def _run(
            self,
            *,
            task: str,
            cmd,
            cwd=None,
            env=None,
            log_path: Path,
            check: bool = True,
    ) -> int:
        self.commands.append([str(item) for item in cmd])
        log_path.parent.mkdir(parents=True, exist_ok=True)
        if not log_path.exists():
            log_path.write_text("", encoding="utf-8")
        return 0

    def _reset_task_log(self, log_path: Path) -> None:
        log_path.parent.mkdir(parents=True, exist_ok=True)
        log_path.write_text("", encoding="utf-8")

    def _clean_path(self, *, task: str, path: Path, log_path: Path) -> None:
        return

    def _stop_host_processes_best_effort(self, *, task: str, log_path: Path) -> None:
        return

    def _stop_host_app_best_effort(self, *, task: str, log_path: Path, app_bin: Path) -> None:
        return


class RecordingXcodeBuildAll(RecordingBuildAll):
    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        self.captured_commands: list[list[str]] = []

    def _capture_with_log(
            self,
            *,
            cmd,
            log_path: Path,
            cwd=None,
            env=None,
    ):
        self.captured_commands.append([str(item) for item in cmd])
        log_path.parent.mkdir(parents=True, exist_ok=True)
        if not log_path.exists():
            log_path.write_text("", encoding="utf-8")
        return SimpleNamespace(returncode=0, stdout="", stderr="")


class RecordingIosTaskBuildAll(RecordingBuildAll):
    def _ensure_ios_device(self, *, task: str, log_path: Path):
        return {
            "identifier": "DEVICE-UDID",
            "name": "Test iPhone",
            "architecture": "arm64",
            "platform": "com.apple.platform.iphoneos",
        }

    def _ensure_ios_lvrs_prefix(self, *, task: str, log_path: Path) -> Path:
        prefix = self.lvrs_prefix / "platforms" / "ios"
        (prefix / "lib" / "cmake" / "LVRS").mkdir(parents=True, exist_ok=True)
        return prefix

    def _uninstall_ios_app_best_effort(self, *, task: str, log_path: Path, device_identifier: str, bundle_id: str) -> None:
        return

    def _ios_backtrace_cmake_args(self, sdk_name: str):
        return []

    def _run(
            self,
            *,
            task: str,
            cmd,
            cwd=None,
            env=None,
            log_path: Path,
            check: bool = True,
    ) -> int:
        self.commands.append([str(item) for item in cmd])
        log_path.parent.mkdir(parents=True, exist_ok=True)
        if not log_path.exists():
            log_path.write_text("", encoding="utf-8")
        if list(cmd[:3]) == ["cmake", "-G", "Xcode"]:
            (self.ios_project_dir / "WhatSon.xcodeproj").mkdir(parents=True, exist_ok=True)
        return 0

    def _run_ios_xcodebuild_with_destination_fallback(
            self,
            *,
            task: str,
            log_path: Path,
            xcode_project: Path,
            derived_data_dir: Path,
            ios_device_id: str,
    ) -> None:
        self.commands.append(
            [
                "xcodebuild",
                "-project",
                str(xcode_project),
                "-destination",
                ios_device_id,
            ]
        )
        app_dir = derived_data_dir / "Build" / "Products" / "Release-iphoneos" / "WhatSon.app"
        app_dir.mkdir(parents=True, exist_ok=True)
        with (app_dir / "Info.plist").open("wb") as fp:
            plistlib.dump({"CFBundleIdentifier": self.ios_bundle_id}, fp)

    def _stage_packaging_artifact(
            self,
            *,
            task: str,
            source: Path,
            destination: Path,
            log_path: Path,
            artifact_kind: str,
    ) -> Path:
        return source

    def _launch_ios_app_with_retry(
            self,
            *,
            task: str,
            log_path: Path,
            xcrun: str,
            ios_device_id: str,
            bundle_id: str,
    ) -> None:
        return


class RecordingRuntimeSmokeMatrix(smoke_matrix.RuntimeSmokeMatrix):
    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        self.commands: list[tuple[str, list[str]]] = []

    def _run(
            self,
            *,
            name: str,
            cmd,
            cwd=None,
            env=None,
            check: bool = True,
    ):
        normalized = [str(item) for item in cmd]
        self.commands.append((name, normalized))
        if name == "ios_xcodebuild":
            app_dir = self.artifacts_dir / "ios-derived-data" / "Build" / "Products" / "Debug-iphonesimulator" / "WhatSon.app"
            app_dir.mkdir(parents=True, exist_ok=True)
        return SimpleNamespace(returncode=0, stdout="", stderr="")

    def _resolve_ios_simulator(self) -> str | None:
        return "SIM-UDID"


def _make_args(temp_root: Path, *, jobs: int, sequential: bool = True) -> argparse.Namespace:
    root = temp_root / "repo"
    root.mkdir(parents=True, exist_ok=True)

    return argparse.Namespace(
        dev_env_json=str(temp_root / "dev_env.json"),
        root=str(root),
        logs_dir=str(temp_root / "logs"),
        tasks="host",
        sequential=sequential,
        parallel=not sequential,
        jobs=jobs,
        no_host_run=True,
        host_build_dir=str(temp_root / "build-host"),
        trial_build_dir=str(temp_root / "build-trial"),
        android_build_dir=str(temp_root / "build-android"),
        ios_project_dir=str(temp_root / "ios-project"),
        android_studio_dir=str(temp_root / "android-studio"),
        qt_version_root=str(temp_root / "qt"),
        qt_host_prefix=str(temp_root / "qt-host"),
        qt_ios_prefix=str(temp_root / "qt-ios"),
        qt_android_prefix=str(temp_root / "qt-android"),
        lvrs_prefix=str(temp_root / "lvrs"),
        lvrs_dir=None,
        android_lvrs_prefix=str(temp_root / "lvrs-android"),
        lvrs_source_dir=str(temp_root / "lvrs-source"),
        skip_android_lvrs_build=False,
        android_sdk_root=str(temp_root / "android-sdk"),
        android_ndk_root=str(temp_root / "android-ndk"),
        android_avd=None,
        android_allow_emulator=True,
        android_package=None,
        ios_bundle_id="com.iisacc.app.whatson",
        ios_device=None,
        ios_development_team=None,
        ios_code_sign_identity=None,
        java21_home=None,
    )


def _make_smoke_args(temp_root: Path, *, jobs: int) -> argparse.Namespace:
    root = temp_root / "repo"
    root.mkdir(parents=True, exist_ok=True)

    return argparse.Namespace(
        root=str(root),
        tasks="host,android,ios",
        logs_dir=str(temp_root / "runtime-logs"),
        artifacts_dir=str(temp_root / "runtime-artifacts"),
        android_package="com.iisacc.app.whatson",
        jobs=jobs,
        ios_bundle_id="com.iisacc.app.whatson",
        host_smoke_seconds=1,
        skip_ios_smoke=False,
        strict_ios_smoke=False,
    )


def _state_payloads(text: str) -> list[dict[str, object]]:
    payloads: list[dict[str, object]] = []
    for line in text.splitlines():
        if not line.startswith("[state] "):
            continue
        payloads.append(json.loads(line[len("[state] "):]))
    return payloads


class BuildParallelismTests(unittest.TestCase):
    def test_parse_apple_development_identities_deduplicates_and_filters(self) -> None:
        output = """
  1) 04800EECDE315EC10E97E25C221A518D1DE10BF1 "Apple Development: MUYEONG YUN (GRWGSK8RDF)"
  2) 0F0387D7FAA466148A18780B4E73333645784E21 "Developer ID Application: MUYEONG YUN (5U49ST9XZH)"
  3) DBD2C38A71240AADDCADD42A5B529063DF892165 "Apple Development: MUYEONG YUN (GRWGSK8RDF)"
  4) 1234567890ABCDEF1234567890ABCDEF12345678 "Apple Development: Example Org (ABCDE12345)"
"""

        identities = runner._parse_apple_development_identities(output)

        self.assertEqual(
            identities,
            [
                runner.AppleDevelopmentIdentity(
                    name="Apple Development: MUYEONG YUN (GRWGSK8RDF)",
                    team_id="GRWGSK8RDF",
                ),
                runner.AppleDevelopmentIdentity(
                    name="Apple Development: Example Org (ABCDE12345)",
                    team_id="ABCDE12345",
                ),
            ],
        )

    def test_emit_state_outputs_json_line(self) -> None:
        buffer = io.StringIO()
        with redirect_stdout(buffer):
            runner.emit_state(
                "unit_test",
                "snapshot",
                root=runner._path_state(REPO_ROOT),
                tasks=["host", "android"],
            )

        payloads = _state_payloads(buffer.getvalue())
        self.assertEqual(len(payloads), 1)
        self.assertEqual(payloads[0]["script"], "unit_test")
        self.assertEqual(payloads[0]["event"], "snapshot")
        self.assertEqual(payloads[0]["root"]["path"], str(REPO_ROOT))

    def test_task_job_limits_distribute_budget(self) -> None:
        limits = runner._task_job_limits(["host", "android", "ios"], 8)
        self.assertEqual(limits, {"host": 3, "android": 3, "ios": 2})

    def test_host_build_uses_explicit_parallel_budget(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=5)
            build_runner = RecordingBuildAll(args)

            result = build_runner.task_host()

        build_commands = [cmd for cmd in build_runner.commands if cmd[:2] == ["cmake", "--build"]]
        self.assertTrue(build_commands)
        self.assertIn("--parallel", build_commands[0])
        self.assertEqual(build_commands[0][-2:], ["--parallel", "5"])
        self.assertNotIn("-j", build_commands[0])
        self.assertEqual(build_commands[0][3:5], ["--target", "WhatSon"])
        self.assertEqual(build_commands[1][3:5], ["--target", "whatson_build_all"])
        self.assertEqual(result.status, "success")

    def test_host_build_also_generates_trial_package_build_dir(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=4)
            build_runner = RecordingBuildAll(args)

            result = build_runner.task_host()

        configure_commands = [cmd for cmd in build_runner.commands if cmd[:3] == ["cmake", "-S", str(build_runner.root)]]
        self.assertTrue(configure_commands)
        self.assertEqual(configure_commands[0][4], str(build_runner.host_build_dir))
        self.assertEqual(configure_commands[1][4], str(build_runner.trial_build_dir))

        build_commands = [cmd for cmd in build_runner.commands if cmd[:2] == ["cmake", "--build"]]
        self.assertTrue(build_commands)
        self.assertEqual(build_commands[2][2], str(build_runner.trial_build_dir))
        self.assertEqual(build_commands[2][3:5], ["--target", "whatson_package"])
        self.assertEqual(result.status, "success")

    def test_linux_headless_host_build_keeps_desktop_app_build_enabled(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=4)
            args.no_host_run = False
            build_runner = RecordingBuildAll(args)
            build_runner.system_name = "Linux"

            with patch.dict("os.environ", {}, clear=True):
                result = build_runner.task_host()

        configure_commands = [cmd for cmd in build_runner.commands if cmd[:3] == ["cmake", "-S", str(build_runner.root)]]
        self.assertTrue(configure_commands)
        self.assertNotIn("-DWHATSON_BUILD_APP=OFF", configure_commands[0])
        self.assertEqual(result.status, "success")
        self.assertIn("skipped desktop app launch", result.detail)

    def test_linux_host_with_display_keeps_desktop_app_enabled(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=4)
            build_runner = RecordingBuildAll(args)
            build_runner.system_name = "Linux"

            with patch.dict("os.environ", {"DISPLAY": ":0"}, clear=True):
                result = build_runner.task_host()

        configure_commands = [cmd for cmd in build_runner.commands if cmd[:3] == ["cmake", "-S", str(build_runner.root)]]
        self.assertTrue(configure_commands)
        self.assertNotIn("-DWHATSON_BUILD_APP=OFF", configure_commands[0])
        self.assertEqual(result.status, "success")

    def test_host_app_binary_prefers_build_root_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=4)
            build_runner = runner.BuildAll(args)
            build_runner.system_name = "Darwin"

            root_app = build_runner.host_build_dir / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon"
            legacy_app = build_runner.host_build_dir / "src" / "app" / "bin" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon"
            root_app.parent.mkdir(parents=True, exist_ok=True)
            legacy_app.parent.mkdir(parents=True, exist_ok=True)
            root_app.write_text("", encoding="utf-8")
            legacy_app.write_text("", encoding="utf-8")

            detected = build_runner._host_app_binary()

        self.assertEqual(detected, root_app)

    def test_android_generic_build_uses_explicit_parallel_budget(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=4)
            build_runner = RecordingBuildAll(args)
            log_path = Path(temp_dir) / "logs" / "android.log"

            detail = build_runner._task_android_generic(task=runner.TASK_ANDROID, log_path=log_path)

        build_commands = [cmd for cmd in build_runner.commands if cmd[:2] == ["cmake", "--build"]]
        self.assertTrue(build_commands)
        self.assertEqual(build_commands[0][-2:], ["--parallel", "4"])
        self.assertEqual(detail, "launch_WhatSon_android completed.")

    def test_ios_xcodebuild_uses_task_parallel_budget(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=6)
            build_runner = RecordingXcodeBuildAll(args)
            build_runner._task_job_limit_map = {runner.TASK_IOS: 3}
            log_path = Path(temp_dir) / "logs" / "ios.log"

            build_runner._run_ios_xcodebuild_with_destination_fallback(
                task=runner.TASK_IOS,
                log_path=log_path,
                xcode_project=Path(temp_dir) / "WhatSon.xcodeproj",
                derived_data_dir=Path(temp_dir) / "derived",
                ios_device_id="DEVICE-UDID",
            )

        self.assertTrue(build_runner.captured_commands)
        self.assertEqual(build_runner.captured_commands[0][:3], ["xcodebuild", "-jobs", "3"])
        self.assertIn("-allowProvisioningUpdates", build_runner.captured_commands[0])
        self.assertIn("-allowProvisioningDeviceRegistration", build_runner.captured_commands[0])

    def test_task_ios_auto_detects_development_team_for_configure(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            args = _make_args(temp_root, jobs=4)
            build_runner = RecordingIosTaskBuildAll(args)
            build_runner.system_name = "Darwin"
            toolchain = Path(args.qt_ios_prefix) / "lib" / "cmake" / "Qt6" / "qt.toolchain.cmake"
            toolchain.parent.mkdir(parents=True, exist_ok=True)
            toolchain.write_text("# mock toolchain\n", encoding="utf-8")

            with patch.object(runner, "_auto_detect_ios_development_team", return_value=("GRWGSK8RDF", [])):
                result = build_runner.task_ios()

        configure_commands = [cmd for cmd in build_runner.commands if cmd[:3] == ["cmake", "-G", "Xcode"]]
        self.assertTrue(configure_commands)
        self.assertIn("-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=GRWGSK8RDF", configure_commands[0])
        self.assertFalse(any(cmd and cmd[0] == "xcodebuild" for cmd in build_runner.commands))
        self.assertEqual(result.status, "success")
        self.assertIn("Generated iOS Xcode project for manual device testing", result.detail)

    def test_gradle_uses_task_parallel_budget(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_args(Path(temp_dir), jobs=6)
            build_runner = RecordingBuildAll(args)
            build_runner._task_job_limit_map = {runner.TASK_ANDROID: 2}

            gradle_args = build_runner._gradle_job_args(runner.TASK_ANDROID)

        self.assertEqual(gradle_args, ["--max-workers", "2", "-Dorg.gradle.parallel=false"])

    def test_clean_path_retries_transient_directory_not_empty_error(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            args = _make_args(temp_root, jobs=2)
            build_runner = runner.BuildAll(args)
            target_dir = temp_root / "transient-clean"
            target_dir.mkdir(parents=True, exist_ok=True)
            (target_dir / "stale.txt").write_text("stale", encoding="utf-8")
            log_path = temp_root / "logs" / "clean.log"
            original_rmtree = runner.shutil.rmtree
            attempts = {"count": 0}

            def flaky_rmtree(path):
                attempts["count"] += 1
                if attempts["count"] == 1:
                    raise OSError(errno.ENOTEMPTY, "Directory not empty", str(path))
                return original_rmtree(path)

            with patch.object(runner.shutil, "rmtree", side_effect=flaky_rmtree):
                with patch.object(runner.time, "sleep", return_value=None):
                    build_runner._clean_path(task=runner.TASK_IOS, path=target_dir, log_path=log_path)

        self.assertEqual(attempts["count"], 2)
        self.assertFalse(target_dir.exists())

    def test_stage_packaging_artifact_copies_directory(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            args = _make_args(temp_root, jobs=2)
            build_runner = runner.BuildAll(args)
            source = temp_root / "derived" / "WhatSon.app"
            destination = build_runner.ios_project_dir / "WhatSon.app"
            log_path = temp_root / "logs" / "stage.log"
            log_path.parent.mkdir(parents=True, exist_ok=True)
            log_path.write_text("", encoding="utf-8")
            (source / "Contents" / "MacOS").mkdir(parents=True, exist_ok=True)
            (source / "Contents" / "MacOS" / "WhatSon").write_text("binary", encoding="utf-8")

            staged = build_runner._stage_packaging_artifact(
                task=runner.TASK_IOS,
                source=source,
                destination=destination,
                log_path=log_path,
                artifact_kind="ios_app_bundle",
            )

            self.assertEqual(staged, destination)
            self.assertTrue((destination / "Contents" / "MacOS" / "WhatSon").is_file())

    def test_build_all_parallel_splits_job_budget(self) -> None:
        recorded: dict[str, int] = {}

        def fake_run_task(*, root: Path, task: str, passthrough_args, jobs: int):
            recorded[task] = jobs
            return build_all_script.TaskResult(task, "success", "completed")

        with patch.object(build_all_script, "_run_task", side_effect=fake_run_task):
            with patch.object(sys, "argv",
                              ["build_all.py", "--tasks", "host,android,ios", "--parallel", "--jobs", "8"]):
                exit_code = build_all_script.main()

        self.assertEqual(exit_code, 0)
        self.assertEqual(recorded, {"host": 3, "android": 3, "ios": 2})

    def test_build_all_forwards_jobs_to_child_script(self) -> None:
        buffer = io.StringIO()
        with patch.object(build_all_script.subprocess, "run", return_value=SimpleNamespace(returncode=0)) as run_mock:
            with redirect_stdout(buffer):
                result = build_all_script._run_task(
                    root=REPO_ROOT,
                    task=build_all_script.TASK_HOST,
                    passthrough_args=["--no-host-run"],
                    jobs=4,
                )

        cmd = run_mock.call_args.args[0]
        jobs_index = cmd.index("--jobs")
        self.assertEqual(cmd[jobs_index + 1], "4")
        self.assertEqual(result.status, "success")
        payloads = _state_payloads(buffer.getvalue())
        events = [payload["event"] for payload in payloads]
        self.assertIn("child_exec_start", events)
        self.assertIn("child_exec_finish", events)

    def test_runtime_smoke_matrix_forwards_jobs_to_build_all(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_smoke_args(Path(temp_dir), jobs=4)
            smoke_runner = RecordingRuntimeSmokeMatrix(args)

            smoke_runner.run_builds()

        build_commands = [cmd for name, cmd in smoke_runner.commands if name == "build_all_matrix"]
        self.assertTrue(build_commands)
        jobs_index = build_commands[0].index("--jobs")
        self.assertEqual(build_commands[0][jobs_index + 1], "4")

    def test_runtime_smoke_matrix_ios_xcodebuild_uses_jobs(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_smoke_args(Path(temp_dir), jobs=5)
            smoke_runner = RecordingRuntimeSmokeMatrix(args)
            smoke_runner.system_name = "darwin"
            project = Path(args.root) / "build" / "ios-xcode-artifact" / "WhatSon.xcodeproj"
            project.mkdir(parents=True, exist_ok=True)

            smoke_runner.ios_simulator_smoke()

        xcode_commands = [cmd for name, cmd in smoke_runner.commands if name == "ios_xcodebuild"]
        self.assertTrue(xcode_commands)
        self.assertEqual(xcode_commands[0][:3], ["xcodebuild", "-jobs", "5"])

    def test_runtime_smoke_matrix_prefers_build_root_host_binary(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            args = _make_smoke_args(Path(temp_dir), jobs=2)
            smoke_runner = smoke_matrix.RuntimeSmokeMatrix(args)
            smoke_runner.system_name = "darwin"
            root_binary = smoke_runner.root / "build" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon"
            legacy_binary = (
                smoke_runner.root / "build" / "src" / "app" / "bin" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon"
            )
            root_binary.parent.mkdir(parents=True, exist_ok=True)
            legacy_binary.parent.mkdir(parents=True, exist_ok=True)
            root_binary.write_text("", encoding="utf-8")
            legacy_binary.write_text("", encoding="utf-8")

            detected = smoke_runner._host_binary()

        self.assertEqual(detected, root_binary)

    def test_dev_env_print_only_emits_state_lines(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            output_dir = Path(temp_dir) / "dev-env-out"
            buffer = io.StringIO()
            argv = [
                "dev_env.py",
                "--repo-root",
                str(REPO_ROOT),
                "--output-dir",
                str(output_dir),
                "--print-only",
            ]
            with patch.object(sys, "argv", argv):
                with redirect_stdout(buffer):
                    exit_code = dev_env_script.main()

        self.assertEqual(exit_code, 0)
        payloads = _state_payloads(buffer.getvalue())
        self.assertTrue(payloads)
        self.assertEqual(payloads[0]["script"], "dev_env")
        events = [payload["event"] for payload in payloads]
        self.assertIn("environment_resolved", events)
        self.assertIn("script_finish", events)

    def test_dev_env_writes_auto_detected_ios_development_team(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            output_dir = Path(temp_dir) / "dev-env-out"
            argv = [
                "dev_env.py",
                "--repo-root",
                str(REPO_ROOT),
                "--output-dir",
                str(output_dir),
            ]
            with patch.object(dev_env_script, "_auto_detect_ios_development_team", return_value=("GRWGSK8RDF", [])):
                with patch.object(sys, "argv", argv):
                    exit_code = dev_env_script.main()

            payload = json.loads((output_dir / "dev_env.json").read_text(encoding="utf-8"))

        self.assertEqual(exit_code, 0)
        self.assertEqual(payload["environment"]["WHATSON_IOS_DEVELOPMENT_TEAM"], "GRWGSK8RDF")


if __name__ == "__main__":
    unittest.main()
