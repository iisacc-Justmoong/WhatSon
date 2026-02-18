use std::collections::HashSet;
use std::env;
use std::path::{Path, PathBuf};
use std::process::{self, Command};

const APP_EXECUTABLES: &[&str] = &[
    "build/src/app/bin/WhatSon.app/Contents/MacOS/WhatSon",
    "build/src/app/bin/WhatSon",
    "build/src/app/WhatSon",
];

fn main() {
    let Some(root) = discover_root() else {
        eprintln!(
            "WhatSon root directory was not found. Run from the repository or set WHATSON_ROOT."
        );
        process::exit(1);
    };

    match launch_prebuilt(&root) {
        Ok(true) => process::exit(0),
        Ok(false) => {}
        Err(err) => {
            eprintln!("Failed to launch prebuilt app: {err}");
        }
    }

    match run_cmake_target(&root) {
        Ok(code) => process::exit(code),
        Err(err) => {
            eprintln!("Failed to run cmake target whatson_run_app: {err}");
            process::exit(1);
        }
    }
}

fn discover_root() -> Option<PathBuf> {
    let mut candidates = Vec::new();

    if let Ok(root) = env::var("WHATSON_ROOT") {
        candidates.push(PathBuf::from(root));
    }

    if let Ok(cwd) = env::current_dir() {
        candidates.extend(cwd.ancestors().map(PathBuf::from));
    }

    let manifest_parent = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../..");
    candidates.push(
        manifest_parent
            .canonicalize()
            .unwrap_or_else(|_| manifest_parent.clone()),
    );

    let mut seen = HashSet::new();
    for path in candidates {
        if !seen.insert(path.clone()) {
            continue;
        }
        if is_project_root(&path) {
            return Some(path);
        }
    }

    None
}

fn is_project_root(path: &Path) -> bool {
    path.join("CMakeLists.txt").is_file() && path.join("src/app/main.cpp").is_file()
}

fn launch_prebuilt(root: &Path) -> Result<bool, std::io::Error> {
    for rel in APP_EXECUTABLES {
        let executable = root.join(rel);
        if executable.is_file() {
            let status = Command::new(&executable).status()?;
            return Ok(status.success());
        }
    }
    Ok(false)
}

fn run_cmake_target(root: &Path) -> Result<i32, std::io::Error> {
    let build_dir = root.join("build");
    if !build_dir.exists() {
        let configure_status = Command::new("cmake")
            .arg("-S")
            .arg(root)
            .arg("-B")
            .arg(&build_dir)
            .status()?;
        if !configure_status.success() {
            return Ok(configure_status.code().unwrap_or(1));
        }
    }

    let build_status = Command::new("cmake")
        .arg("--build")
        .arg(&build_dir)
        .arg("--target")
        .arg("whatson_run_app")
        .status()?;

    Ok(build_status.code().unwrap_or(1))
}
