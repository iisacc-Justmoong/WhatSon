#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import shlex
import shutil
import subprocess
from typing import Iterable, List, Optional, Sequence, Tuple

import sys
import time
from pathlib import Path

from build_platform_runner import _default_build_jobs, _path_state, _positive_int_arg, _preview_text, emit_state


class SmokeError(RuntimeError):
    pass


def _expand(path: str) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(path))).resolve()


def _quote(cmd: Sequence[str]) -> str:
    return " ".join(shlex.quote(str(item)) for item in cmd)


class RuntimeSmokeMatrix:
    def __init__(self, args: argparse.Namespace) -> None:
        self.root = _expand(args.root)
        self.tasks = [item.strip() for item in args.tasks.split(",") if item.strip()]
        self.logs_dir = _expand(args.logs_dir)
        self.artifacts_dir = _expand(args.artifacts_dir)
        self.logs_dir.mkdir(parents=True, exist_ok=True)
        self.artifacts_dir.mkdir(parents=True, exist_ok=True)

        self.android_package = args.android_package
        self.ios_bundle_id = args.ios_bundle_id
        self.host_smoke_seconds = max(1, int(args.host_smoke_seconds))
        self.skip_ios_smoke = args.skip_ios_smoke
        self.strict_ios_smoke = args.strict_ios_smoke
        self.jobs = args.jobs
        self.build_all_path = _expand(str(self.root / "scripts" / "build_all.py"))

        self.system_name = sys.platform

    def _emit_state(self, event: str, **fields) -> None:
        emit_state("runtime_smoke_matrix", event, **fields)

    def _run(
            self,
            *,
            name: str,
            cmd: Sequence[str],
            cwd: Optional[Path] = None,
            env: Optional[dict] = None,
            check: bool = True,
    ) -> subprocess.CompletedProcess[str]:
        log_path = self.logs_dir / f"{name}.log"
        merged_env = os.environ.copy()
        if env:
            merged_env.update(env)
        cmd_list = [str(item) for item in cmd]
        cmd_text = _quote(cmd)
        started_at = time.monotonic()
        self._emit_state(
            "command_start",
            name=name,
            cmd=cmd_list,
            cmd_text=cmd_text,
            cwd=_path_state(cwd or self.root),
            log_path=_path_state(log_path),
        )

        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"$ {cmd_text}\n")
            fp.write(f"# cwd: {cwd or self.root}\n")
            proc = subprocess.run(
                cmd_list,
                cwd=str(cwd or self.root),
                env=merged_env,
                stdout=fp,
                stderr=subprocess.STDOUT,
                text=True,
                check=False,
            )
            fp.write(f"[exit] {proc.returncode}\n\n")

        self._emit_state(
            "command_finish",
            name=name,
            cmd=cmd_list,
            cmd_text=cmd_text,
            returncode=proc.returncode,
            duration_seconds=round(time.monotonic() - started_at, 3),
            log_path=_path_state(log_path),
        )

        if check and proc.returncode != 0:
            raise SmokeError(f"{name} failed ({proc.returncode}): {cmd_text}")
        return proc

    def _capture(
            self,
            *,
            cmd: Sequence[str],
            cwd: Optional[Path] = None,
            env: Optional[dict] = None,
    ) -> subprocess.CompletedProcess[str]:
        merged_env = os.environ.copy()
        if env:
            merged_env.update(env)
        cmd_list = [str(item) for item in cmd]
        cmd_text = _quote(cmd)
        started_at = time.monotonic()
        self._emit_state(
            "capture_start",
            cmd=cmd_list,
            cmd_text=cmd_text,
            cwd=_path_state(cwd or self.root),
        )
        process = subprocess.run(
            cmd_list,
            cwd=str(cwd or self.root),
            env=merged_env,
            capture_output=True,
            text=True,
            check=False,
        )
        self._emit_state(
            "capture_finish",
            cmd=cmd_list,
            cmd_text=cmd_text,
            returncode=process.returncode,
            duration_seconds=round(time.monotonic() - started_at, 3),
            stdout_preview=_preview_text(process.stdout),
            stderr_preview=_preview_text(process.stderr),
        )
        return process

    def _find_existing(self, candidates: Iterable[Path]) -> Optional[Path]:
        for path in candidates:
            if path.exists():
                return path
        return None

    def _host_binary(self) -> Optional[Path]:
        if self.system_name.startswith("darwin"):
            candidates = [
                self.root / "build" / "host-auto" / "src" / "app" / "bin" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon",
                self.root / "build" / "src" / "app" / "bin" / "WhatSon.app" / "Contents" / "MacOS" / "WhatSon",
            ]
        elif self.system_name.startswith("win"):
            candidates = [
                self.root / "build" / "host-auto" / "src" / "app" / "bin" / "WhatSon.exe",
                self.root / "build" / "src" / "app" / "bin" / "WhatSon.exe",
            ]
        else:
            candidates = [
                self.root / "build" / "host-auto" / "src" / "app" / "bin" / "WhatSon",
                self.root / "build" / "src" / "app" / "bin" / "WhatSon",
            ]
        return self._find_existing(candidates)

    def _find_adb(self) -> Optional[Path]:
        from_path = shutil.which("adb")
        if from_path:
            return _expand(from_path)
        sdk_root = _expand(os.environ.get("ANDROID_SDK_ROOT", str(_expand("~") / "Library" / "Android" / "sdk")))
        candidates = [sdk_root / "platform-tools" / "adb"]
        if self.system_name.startswith("win"):
            candidates.insert(0, sdk_root / "platform-tools" / "adb.exe")
        return self._find_existing(candidates)

    def _resolve_ios_simulator(self) -> Optional[str]:
        result = self._capture(cmd=["xcrun", "simctl", "list", "devices", "available", "-j"])
        if result.returncode != 0:
            return None
        try:
            payload = json.loads(result.stdout)
        except json.JSONDecodeError:
            return None
        devices = payload.get("devices", {})
        booted: List[Tuple[str, str]] = []
        shutdown: List[Tuple[str, str]] = []
        for _runtime, entries in devices.items():
            if not isinstance(entries, list):
                continue
            for item in entries:
                if not isinstance(item, dict):
                    continue
                if not item.get("isAvailable", False):
                    continue
                name = str(item.get("name", ""))
                udid = str(item.get("udid", ""))
                state = str(item.get("state", ""))
                if not udid:
                    continue
                if "iPhone" not in name and "iPad" not in name:
                    continue
                if state == "Booted":
                    booted.append((name, udid))
                else:
                    shutdown.append((name, udid))
        selected = booted[0] if booted else (shutdown[0] if shutdown else None)
        return selected[1] if selected else None

    def run_builds(self) -> None:
        self._emit_state("phase_start", phase="run_builds", tasks=self.tasks, jobs=self.jobs)
        cmd = [
            "python3",
            str(self.build_all_path),
            "--tasks",
            ",".join(self.tasks),
            "--sequential",
            "--jobs",
            str(self.jobs),
            "--no-host-run",
        ]
        self._run(name="build_all_matrix", cmd=cmd)
        self._emit_state("phase_finish", phase="run_builds", status="success")

    def verify_qml_cache_contract(self) -> None:
        self._emit_state("phase_start", phase="verify_qml_cache_contract")
        required = [
            self.root / "build" / "host-auto" / "src" / "app" / ".rcc" / "qmlcache" / "WhatSon_qml" / "shell" / "BodyLayout_qml.cpp",
            self.root / "build" / "android-auto" / "src" / "app" / ".rcc" / "qmlcache" / "WhatSon_qml" / "shell" / "BodyLayout_qml.cpp",
        ]
        missing = [str(path) for path in required if not path.exists()]
        if missing:
            self._emit_state("phase_finish", phase="verify_qml_cache_contract", status="failed", missing=missing)
            raise SmokeError("QML shell layout cache missing: " + ", ".join(missing))
        self._emit_state("phase_finish", phase="verify_qml_cache_contract", status="success",
                         checked=[str(path) for path in required])

    def host_smoke(self) -> None:
        if "host" not in self.tasks:
            return
        self._emit_state("phase_start", phase="host_smoke")
        app = self._host_binary()
        if app is None:
            self._emit_state("phase_finish", phase="host_smoke", status="failed",
                             detail="Host app binary was not found after build.")
            raise SmokeError("Host app binary was not found after build.")

        log_path = self.logs_dir / "host_runtime.log"
        with log_path.open("a", encoding="utf-8") as fp:
            fp.write(f"$ launch {_quote([str(app)])}\n")
            process = subprocess.Popen(
                [str(app)],
                cwd=str(self.root),
                stdout=fp,
                stderr=subprocess.STDOUT,
                start_new_session=not self.system_name.startswith("win"),
            )
            fp.write(f"# pid={process.pid}\n")

        time.sleep(self.host_smoke_seconds)
        if process.poll() is not None and process.returncode not in (0, None):
            self._emit_state(
                "phase_finish",
                phase="host_smoke",
                status="failed",
                app=_path_state(app),
                pid=process.pid,
                returncode=process.returncode,
            )
            raise SmokeError(f"Host runtime exited unexpectedly (code={process.returncode}).")

        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=5)
        self._emit_state("phase_finish", phase="host_smoke", status="success", app=_path_state(app), pid=process.pid)

    def android_smoke(self) -> None:
        if "android" not in self.tasks:
            return
        self._emit_state("phase_start", phase="android_smoke", android_package=self.android_package)
        adb = self._find_adb()
        if adb is None:
            self._emit_state("phase_finish", phase="android_smoke", status="failed",
                             detail="adb was not found for Android runtime smoke.")
            raise SmokeError("adb was not found for Android runtime smoke.")

        packages = self._capture(cmd=[str(adb), "shell", "pm", "list", "packages"])
        if packages.returncode != 0 or self.android_package not in packages.stdout:
            self._emit_state("phase_finish", phase="android_smoke", status="failed",
                             detail=f"Android package not installed: {self.android_package}")
            raise SmokeError(f"Android package not installed: {self.android_package}")
        if "org.qtproject.example.WhatSon" in packages.stdout:
            self._emit_state("phase_finish", phase="android_smoke", status="failed",
                             detail="Legacy package org.qtproject.example.WhatSon is still installed.")
            raise SmokeError("Legacy package org.qtproject.example.WhatSon is still installed.")

        dumpsys = self._capture(cmd=[str(adb), "shell", "dumpsys", "activity", "activities"])
        if dumpsys.returncode != 0:
            self._emit_state("phase_finish", phase="android_smoke", status="failed",
                             detail="Failed to read Android activity state.")
            raise SmokeError("Failed to read Android activity state.")
        resumed_key = f"mResumedActivity" in dumpsys.stdout or "topResumedActivity" in dumpsys.stdout
        package_visible = self.android_package in dumpsys.stdout
        if not resumed_key or not package_visible:
            self._emit_state("phase_finish", phase="android_smoke", status="failed",
                             detail="Android resumed activity does not match the expected package.")
            raise SmokeError("Android resumed activity does not match the expected package.")

        screenshot_path = self.artifacts_dir / "android-runtime.png"
        with screenshot_path.open("wb") as fp:
            shot = subprocess.run(
                [str(adb), "exec-out", "screencap", "-p"],
                cwd=str(self.root),
                stdout=fp,
                stderr=subprocess.PIPE,
                check=False,
            )
        if shot.returncode != 0:
            self._emit_state("phase_finish", phase="android_smoke", status="failed",
                             detail="Failed to capture Android runtime screenshot.")
            raise SmokeError("Failed to capture Android runtime screenshot.")
        self._emit_state(
            "phase_finish",
            phase="android_smoke",
            status="success",
            adb_path=_path_state(adb),
            screenshot_path=_path_state(screenshot_path),
        )

    def ios_simulator_smoke(self) -> None:
        if "ios" not in self.tasks:
            return
        if self.skip_ios_smoke:
            self._emit_state("phase_finish", phase="ios_simulator_smoke", status="skipped",
                             detail="skip_ios_smoke flag")
            return
        if not self.system_name.startswith("darwin"):
            self._emit_state("phase_finish", phase="ios_simulator_smoke", status="skipped", detail="non-darwin host")
            return
        self._emit_state("phase_start", phase="ios_simulator_smoke", ios_bundle_id=self.ios_bundle_id)

        project = self.root / "build" / "ios-xcode-artifact" / "WhatSon.xcodeproj"
        if not project.exists():
            self._emit_state("phase_finish", phase="ios_simulator_smoke", status="failed", project=_path_state(project))
            raise SmokeError(f"iOS project is missing: {project}")

        udid = self._resolve_ios_simulator()
        if not udid:
            self._emit_state("phase_finish", phase="ios_simulator_smoke", status="failed",
                             detail="No available iOS simulator was found.")
            raise SmokeError("No available iOS simulator was found.")

        self._run(name="ios_sim_boot", cmd=["xcrun", "simctl", "boot", udid], check=False)
        self._run(name="ios_sim_bootstatus", cmd=["xcrun", "simctl", "bootstatus", udid, "-b"])

        derived = self.artifacts_dir / "ios-derived-data"
        if derived.exists():
            shutil.rmtree(derived)

        build_cmd = [
            "xcodebuild",
            "-jobs",
            str(self.jobs),
            "-project",
            str(project),
            "-scheme",
            "WhatSon",
            "-configuration",
            "Debug",
            "-destination",
            f"id={udid}",
            "-derivedDataPath",
            str(derived),
            "build",
            "CODE_SIGNING_ALLOWED=NO",
            "CODE_SIGNING_REQUIRED=NO",
        ]
        build_result = self._run(name="ios_xcodebuild", cmd=build_cmd, check=False)
        if build_result.returncode != 0:
            log_text = (self.logs_dir / "ios_xcodebuild.log").read_text(encoding="utf-8", errors="ignore")
            kit_mismatch = "building for 'iOS-simulator'" in log_text and "built for 'iOS'" in log_text
            if kit_mismatch and not self.strict_ios_smoke:
                print("[runtime-matrix] ios smoke skipped: Qt iOS simulator slice mismatch in current Qt kit")
                self._emit_state("phase_finish", phase="ios_simulator_smoke", status="skipped",
                                 detail="Qt iOS simulator slice mismatch")
                return
            self._emit_state(
                "phase_finish",
                phase="ios_simulator_smoke",
                status="failed",
                returncode=build_result.returncode,
                project=_path_state(project),
            )
            raise SmokeError(f"ios_xcodebuild failed ({build_result.returncode}): {_quote(build_cmd)}")

        app = derived / "Build" / "Products" / "Debug-iphonesimulator" / "WhatSon.app"
        if not app.exists():
            self._emit_state("phase_finish", phase="ios_simulator_smoke", status="failed", app=_path_state(app))
            raise SmokeError(f"Built iOS simulator app is missing: {app}")

        self._run(
            name="ios_sim_uninstall",
            cmd=["xcrun", "simctl", "uninstall", udid, self.ios_bundle_id],
            check=False,
        )
        self._run(name="ios_sim_install", cmd=["xcrun", "simctl", "install", udid, str(app)])
        self._run(name="ios_sim_launch", cmd=["xcrun", "simctl", "launch", udid, self.ios_bundle_id])

        screenshot_path = self.artifacts_dir / "ios-runtime.png"
        self._run(name="ios_sim_screenshot", cmd=["xcrun", "simctl", "io", udid, "screenshot", str(screenshot_path)])
        self._emit_state(
            "phase_finish",
            phase="ios_simulator_smoke",
            status="success",
            project=_path_state(project),
            screenshot_path=_path_state(screenshot_path),
            simulator_udid=udid,
        )

    def run(self) -> None:
        self._emit_state(
            "script_start",
            root=_path_state(self.root),
            tasks=self.tasks,
            logs_dir=_path_state(self.logs_dir),
            artifacts_dir=_path_state(self.artifacts_dir),
            jobs=self.jobs,
            android_package=self.android_package,
            ios_bundle_id=self.ios_bundle_id,
        )
        self.run_builds()
        self.verify_qml_cache_contract()
        self.host_smoke()
        self.android_smoke()
        self.ios_simulator_smoke()
        self._emit_state("script_finish", status="success")


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(
        description="Cross-platform runtime smoke matrix for WhatSon (host/android/ios)."
    )
    parser.add_argument("--root", default=str(root), help="Repository root path.")
    parser.add_argument("--tasks", default="host,android,ios", help="Comma-separated tasks: host,android,ios")
    parser.add_argument(
        "--logs-dir",
        default=str(root / "build" / "runtime-matrix-logs"),
        help="Runtime matrix log directory.",
    )
    parser.add_argument(
        "--artifacts-dir",
        default=str(root / "build" / "runtime-matrix-artifacts"),
        help="Runtime matrix artifacts directory.",
    )
    parser.add_argument(
        "--android-package",
        default=os.environ.get("WHATSON_ANDROID_PACKAGE", "com.iisacc.app.whatson"),
        help="Expected Android package id.",
    )
    parser.add_argument(
        "--jobs",
        type=_positive_int_arg,
        default=_default_build_jobs(),
        help="Maximum native build jobs to use while invoking build_all.py and iOS simulator xcodebuild.",
    )
    parser.add_argument(
        "--ios-bundle-id",
        default=os.environ.get("WHATSON_APPLE_BUNDLE_ID", "com.iisacc.app.whatson"),
        help="Expected iOS bundle id.",
    )
    parser.add_argument("--host-smoke-seconds", default=4, type=int, help="Seconds to keep host app running.")
    parser.add_argument("--skip-ios-smoke", action="store_true", help="Skip iOS simulator launch smoke.")
    parser.add_argument(
        "--strict-ios-smoke",
        action="store_true",
        help="Fail runtime matrix when iOS simulator smoke cannot run due Qt kit mismatch.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    runner = RuntimeSmokeMatrix(args)
    print(f"[runtime-matrix] root={runner.root}", flush=True)
    print(f"[runtime-matrix] tasks={','.join(runner.tasks)}", flush=True)
    print(f"[runtime-matrix] logs={runner.logs_dir}", flush=True)
    print(f"[runtime-matrix] artifacts={runner.artifacts_dir}", flush=True)
    print(f"[runtime-matrix] jobs={runner.jobs}", flush=True)
    try:
        runner.run()
    except Exception as exc:  # noqa: BLE001
        emit_state("runtime_smoke_matrix", "script_finish", status="failed", detail=str(exc))
        print(f"[runtime-matrix] failed: {exc}", flush=True)
        return 1
    print("[runtime-matrix] success", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
