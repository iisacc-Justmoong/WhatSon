#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shlex
import shutil
import subprocess
import textwrap
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

import sys

from build_platform_runner import _path_state, emit_state


def _expand(value: str | Path) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(str(value)))).resolve()


def _run_capture(cmd: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, capture_output=True, text=True, check=False)


def _latest_version_dir(parent: Path, pattern: str = r"^\d+(\.\d+)*$") -> Optional[Path]:
    if not parent.is_dir():
        return None
    regex = re.compile(pattern)
    candidates = [item for item in parent.iterdir() if item.is_dir() and regex.match(item.name)]
    if not candidates:
        return None

    def sort_key(item: Path) -> Tuple[int, ...]:
        return tuple(int(part) for part in re.findall(r"\d+", item.name))

    return sorted(candidates, key=sort_key)[-1]


def _first_existing(paths: Iterable[Path]) -> Optional[Path]:
    for path in paths:
        if path.exists():
            return path
    return None


def _default_qt_version_root(home: Path) -> Path:
    qt_home = home / "Qt"
    return _latest_version_dir(qt_home) or qt_home


def _default_qt_host_prefix(system_name: str, qt_version_root: Path) -> Path:
    if system_name == "Darwin":
        return qt_version_root / "macos"
    if system_name == "Windows":
        return _first_existing([
            qt_version_root / "msvc2022_64",
            qt_version_root / "msvc2019_64",
            qt_version_root / "mingw_64",
        ]) or (qt_version_root / "msvc2022_64")
    return qt_version_root / "gcc_64"


def _default_lvrs_source_dir(home: Path, repo_root: Path) -> Optional[Path]:
    candidates = [
        home / "Developer" / "LVRS",
        repo_root.parent / "LVRS",
        repo_root.parent.parent / "LVRS",
        Path("/local/LVRS"),
    ]
    for path in candidates:
        if (path / "CMakeLists.txt").exists():
            return path
    return None


def _resolve_lvrs_prefix(home: Path, repo_root: Path) -> Path:
    env_prefix = os.environ.get("LVRS_PREFIX")
    if env_prefix:
        return _expand(env_prefix)
    candidates = [home / ".local" / "LVRS", Path("/local/LVRS"), repo_root.parent / "LVRS"]
    return _first_existing(candidates) or candidates[0]


def _resolve_lvrs_dir(lvrs_prefix: Path, system_name: str) -> Path:
    root_dispatch = lvrs_prefix / "blueprint" / "cmake" / "LVRS"
    if root_dispatch.exists():
        return root_dispatch

    platform_key = {
        "Darwin": "macos",
        "Linux": "linux",
        "Windows": "windows",
    }.get(system_name, "macos")
    platform_dir = lvrs_prefix / "platforms" / platform_key / "blueprint" / "cmake" / "LVRS"
    if platform_dir.exists():
        return platform_dir

    root_fallback = lvrs_prefix / "lib" / "cmake" / "LVRS"
    if root_fallback.exists():
        return root_fallback

    platform_fallback = lvrs_prefix / "platforms" / platform_key / "lib" / "cmake" / "LVRS"
    if platform_fallback.exists():
        return platform_fallback
    return root_dispatch


def _cmake_version_ok() -> Tuple[bool, str]:
    cmake_bin = shutil.which("cmake")
    if not cmake_bin:
        return False, "cmake was not found in PATH."
    probe = _run_capture([cmake_bin, "--version"])
    if probe.returncode != 0:
        return False, "cmake exists but version probe failed."
    match = re.search(r"cmake version (\d+)\.(\d+)\.(\d+)", probe.stdout)
    if not match:
        return False, "cmake version could not be parsed."
    major, minor, _ = (int(value) for value in match.groups())
    if (major, minor) < (3, 24):
        return False, f"cmake>=3.24 is required, found {match.group(0).split()[-1]}."
    return True, ""


def _build_manual_actions(env_map: Dict[str, str], tools: Dict[str, bool]) -> List[str]:
    actions: List[str] = []
    cmake_ok, cmake_msg = _cmake_version_ok()
    if not cmake_ok:
        actions.append(f"Install/upgrade CMake 3.24+ ({cmake_msg})")
    if not tools["python3"]:
        actions.append("Install Python 3 and ensure python3 is available in PATH.")
    if not tools["git"]:
        actions.append("Install Git and ensure git is available in PATH.")
    if not tools["cargo"]:
        actions.append("Install Rust toolchain (rustup + cargo) for src/cli build target.")
    qt_host_prefix = Path(env_map["QT_HOST_PREFIX"])
    if not qt_host_prefix.exists():
        actions.append(f"Install Qt host kit or set QT_HOST_PREFIX. Expected path: {qt_host_prefix}")
    lvrs_dir = Path(env_map["LVRS_DIR"])
    if not lvrs_dir.exists():
        actions.append(f"Install LVRS package and/or set LVRS_PREFIX. Expected LVRS_DIR path: {lvrs_dir}")
    return actions


def _write_json(path: Path, payload: Dict[str, object]) -> None:
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def _write_shell(path: Path, env_map: Dict[str, str], generated_at: str) -> None:
    lines = [
        "#!/usr/bin/env bash",
        "set -euo pipefail",
        f"# Generated by dev_env.py at {generated_at}",
        "",
    ]
    for key in sorted(env_map.keys()):
        value = env_map[key]
        if value:
            lines.append(f"export {key}={shlex.quote(value)}")
    lines.append("")
    lines.append('echo "[whatson-dev-env] environment loaded from build/dev-env/dev_env.sh"')
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    path.chmod(0o755)


def _write_build_all_wrapper(path: Path, repo_root: Path, env_sh_path: Path) -> None:
    script = textwrap.dedent(
        f"""\
        #!/usr/bin/env bash
        set -euo pipefail
        source {shlex.quote(str(env_sh_path))}
        exec python3 {shlex.quote(str(repo_root / "scripts" / "build_all.py"))} "$@"
        """
    )
    path.write_text(script, encoding="utf-8")
    path.chmod(0o755)


def _write_brief(path: Path, env_sh_path: Path, build_all_wrapper_path: Path, manual_actions: List[str]) -> None:
    lines = [
        "WhatSon Development Environment",
        "================================",
        "",
        f"1) Load environment: source {env_sh_path}",
        f"2) Run host build orchestrator: {build_all_wrapper_path} --tasks host",
        "",
    ]
    if manual_actions:
        lines.append("Manual actions required:")
        for idx, action in enumerate(manual_actions, start=1):
            lines.append(f"{idx}. {action}")
    else:
        lines.append("Manual actions required: none")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Normalize WhatSon desktop development environment for this machine.")
    parser.add_argument("--repo-root", default=str(repo_root), help="WhatSon repository root.")
    parser.add_argument("--output-dir", default=str(repo_root / "build" / "dev-env"))
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--print-only", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = _expand(args.repo_root)
    output_dir = _expand(args.output_dir)
    system_name = platform.system()
    home = _expand("~")
    emit_state(
        "dev_env",
        "script_start",
        repo_root=_path_state(repo_root),
        output_dir=_path_state(output_dir),
        strict=args.strict,
        print_only=args.print_only,
        system=system_name,
    )

    qt_version_root = _expand(os.environ.get("QT_VERSION_ROOT", str(_default_qt_version_root(home))))
    qt_host_prefix = _expand(os.environ.get("QT_HOST_PREFIX", str(_default_qt_host_prefix(system_name, qt_version_root))))
    lvrs_prefix = _resolve_lvrs_prefix(home, repo_root)
    lvrs_dir = _resolve_lvrs_dir(lvrs_prefix, system_name)
    lvrs_source_dir = _default_lvrs_source_dir(home, repo_root)

    env_map: Dict[str, str] = {
        "WHATSON_ROOT": str(repo_root),
        "QT_VERSION_ROOT": str(qt_version_root),
        "QT_HOST_PREFIX": str(qt_host_prefix),
        "LVRS_PREFIX": str(lvrs_prefix),
        "LVRS_DIR": str(lvrs_dir),
    }
    if lvrs_source_dir:
        env_map["LVRS_SOURCE_DIR"] = str(lvrs_source_dir)

    tools = {
        "python3": shutil.which("python3") is not None,
        "git": shutil.which("git") is not None,
        "cargo": shutil.which("cargo") is not None,
    }
    manual_actions = _build_manual_actions(env_map, tools)
    emit_state(
        "dev_env",
        "environment_resolved",
        repo_root=_path_state(repo_root),
        output_dir=_path_state(output_dir),
        qt_version_root=_path_state(qt_version_root),
        qt_host_prefix=_path_state(qt_host_prefix),
        lvrs_prefix=_path_state(lvrs_prefix),
        lvrs_dir=_path_state(lvrs_dir),
        lvrs_source_dir=_path_state(lvrs_source_dir),
        tools=tools,
        manual_actions=manual_actions,
    )

    if not args.print_only:
        output_dir.mkdir(parents=True, exist_ok=True)
        json_path = output_dir / "dev_env.json"
        sh_path = output_dir / "dev_env.sh"
        build_all_wrapper_path = output_dir / "build_all.sh"
        brief_path = output_dir / "README.txt"
        generated_at = _run_capture(["date", "+%Y-%m-%dT%H:%M:%S%z"]).stdout.strip() or "unknown"
        payload: Dict[str, object] = {
            "repo_root": str(repo_root),
            "system": system_name,
            "environment": env_map,
            "tool_check": tools,
            "manual_actions": manual_actions,
        }
        _write_json(json_path, payload)
        _write_shell(sh_path, env_map, generated_at)
        _write_build_all_wrapper(build_all_wrapper_path, repo_root, sh_path)
        _write_brief(brief_path, sh_path, build_all_wrapper_path, manual_actions)
        emit_state(
            "dev_env",
            "artifacts_written",
            json_path=_path_state(json_path),
            shell_path=_path_state(sh_path),
            build_all_wrapper_path=_path_state(build_all_wrapper_path),
            brief_path=_path_state(brief_path),
        )

    print(f"[dev_env] repo_root={repo_root}", flush=True)
    print(f"[dev_env] output_dir={output_dir}", flush=True)
    print("[dev_env] resolved environment:", flush=True)
    for key in sorted(env_map.keys()):
        print(f"  - {key}={env_map[key]}", flush=True)

    if manual_actions:
        print("[dev_env] manual actions required:", flush=True)
        for idx, action in enumerate(manual_actions, start=1):
            print(f"  {idx}. {action}", flush=True)
    else:
        print("[dev_env] manual actions required: none", flush=True)

    if args.strict and manual_actions:
        emit_state("dev_env", "script_finish", status="failed", exit_code=2, manual_actions=manual_actions)
        return 2
    emit_state("dev_env", "script_finish", status="success", exit_code=0, manual_actions=manual_actions)
    return 0


if __name__ == "__main__":
    sys.exit(main())
