use std::collections::HashSet;
use std::env;
use std::fs;
use std::io::{self, ErrorKind};
use std::path::{Path, PathBuf};
use std::process::{self, Command};

const APP_EXECUTABLES: &[&str] = &[
    "build/host-auto/src/app/bin/WhatSon.app/Contents/MacOS/WhatSon",
    "build/host-auto/src/app/bin/WhatSon",
    "build/host-auto/src/app/WhatSon",
    "build/src/app/bin/WhatSon.app/Contents/MacOS/WhatSon",
    "build/src/app/bin/WhatSon",
    "build/src/app/WhatSon",
];

const BUILD_DIRS: &[&str] = &["build/host-auto", "build"];

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum LaunchMode {
    Workspace,
    OnboardingOnly,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum CliAction {
    Launch(LaunchMode),
    PrintHelp,
}

fn main() {
    let cli_args: Vec<String> = env::args().skip(1).collect();
    let action = match parse_cli_action(&cli_args) {
        Ok(action) => action,
        Err(message) => {
            eprintln!("{message}");
            process::exit(2);
        }
    };
    if action == CliAction::PrintHelp {
        println!("{}", usage_text());
        process::exit(0);
    }
    let launch_mode = match action {
        CliAction::Launch(mode) => mode,
        CliAction::PrintHelp => unreachable!(),
    };

    let Some(root) = discover_root() else {
        eprintln!(
            "WhatSon root directory was not found. Run from the repository or set WHATSON_ROOT."
        );
        process::exit(1);
    };

    if should_try_prebuilt(launch_mode) {
        match try_launch_prebuilt(&root, launch_mode) {
            Ok(Some(code)) => process::exit(code),
            Ok(None) => {}
            Err(err) => {
                eprintln!("Failed to launch prebuilt app: {err}");
            }
        }
    }

    match build_and_launch(&root, launch_mode) {
        Ok(code) => process::exit(code),
        Err(err) => {
            eprintln!("Failed to build and launch WhatSon: {err}");
            process::exit(1);
        }
    }
}

fn usage_text() -> &'static str {
    "Usage: whatson [onboard]\n\nCommands:\n  onboard    Launch only the onboarding window."
}

fn parse_cli_action(args: &[String]) -> Result<CliAction, String> {
    match args {
        [] => Ok(CliAction::Launch(LaunchMode::Workspace)),
        [command] if command == "onboard" => Ok(CliAction::Launch(LaunchMode::OnboardingOnly)),
        [command] if command == "-h" || command == "--help" || command == "help" => {
            Ok(CliAction::PrintHelp)
        }
        [command] => Err(format!("Unknown command: {command}\n\n{}", usage_text())),
        _ => Err(format!("Too many arguments.\n\n{}", usage_text())),
    }
}

fn should_try_prebuilt(mode: LaunchMode) -> bool {
    matches!(mode, LaunchMode::Workspace)
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

fn app_arguments(mode: LaunchMode) -> &'static [&'static str] {
    match mode {
        LaunchMode::Workspace => &[],
        LaunchMode::OnboardingOnly => &["--onboarding-only"],
    }
}

fn try_launch_prebuilt(root: &Path, mode: LaunchMode) -> Result<Option<i32>, io::Error> {
    for rel in APP_EXECUTABLES {
        let executable = root.join(rel);
        if executable.is_file() {
            let status = Command::new(&executable)
                .args(app_arguments(mode))
                .status()?;
            return Ok(Some(status.code().unwrap_or(1)));
        }
    }
    Ok(None)
}

fn build_and_launch(root: &Path, mode: LaunchMode) -> Result<i32, io::Error> {
    let build_dir = pick_cmake_build_dir(root);
    if build_cache_exists(&build_dir) && !build_cache_matches_root(&build_dir, root) {
        clear_stale_cmake_cache(&build_dir)?;
    }
    if !build_cache_exists(&build_dir) {
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
        .arg("WhatSon")
        .status()?;

    if !build_status.success() {
        return Ok(build_status.code().unwrap_or(1));
    }

    match try_launch_prebuilt(root, mode)? {
        Some(code) => Ok(code),
        None => Err(io::Error::new(
            ErrorKind::NotFound,
            format!(
                "Built WhatSon target, but no executable was found under {}",
                build_dir.display()
            ),
        )),
    }
}

fn pick_cmake_build_dir(root: &Path) -> PathBuf {
    for rel in BUILD_DIRS {
        let candidate = root.join(rel);
        if build_cache_matches_root(&candidate, root) {
            return candidate;
        }
    }
    root.join(BUILD_DIRS[0])
}

fn build_cache_exists(build_dir: &Path) -> bool {
    build_dir.join("CMakeCache.txt").is_file()
}

fn build_cache_matches_root(build_dir: &Path, root: &Path) -> bool {
    let cache_path = build_dir.join("CMakeCache.txt");
    let Ok(cache_text) = fs::read_to_string(cache_path) else {
        return false;
    };
    let Some(home_dir) = extract_cmake_home_directory(&cache_text) else {
        return false;
    };
    paths_match(Path::new(home_dir), root)
}

fn extract_cmake_home_directory(cache_text: &str) -> Option<&str> {
    cache_text
        .lines()
        .find_map(|line| line.strip_prefix("CMAKE_HOME_DIRECTORY:INTERNAL="))
}

fn paths_match(left: &Path, right: &Path) -> bool {
    let left = left.canonicalize().unwrap_or_else(|_| left.to_path_buf());
    let right = right.canonicalize().unwrap_or_else(|_| right.to_path_buf());
    left == right
}

fn clear_stale_cmake_cache(build_dir: &Path) -> Result<(), io::Error> {
    remove_file_if_exists(&build_dir.join("CMakeCache.txt"))?;
    remove_file_if_exists(&build_dir.join("build.ninja"))?;
    remove_file_if_exists(&build_dir.join("Makefile"))?;
    remove_file_if_exists(&build_dir.join("cmake_install.cmake"))?;
    remove_dir_if_exists(&build_dir.join("CMakeFiles"))?;
    Ok(())
}

fn remove_file_if_exists(path: &Path) -> Result<(), io::Error> {
    match fs::remove_file(path) {
        Ok(()) => Ok(()),
        Err(err) if err.kind() == ErrorKind::NotFound => Ok(()),
        Err(err) => Err(err),
    }
}

fn remove_dir_if_exists(path: &Path) -> Result<(), io::Error> {
    match fs::remove_dir_all(path) {
        Ok(()) => Ok(()),
        Err(err) if err.kind() == ErrorKind::NotFound => Ok(()),
        Err(err) => Err(err),
    }
}

#[cfg(test)]
mod tests {
    fn args(values: &[&str]) -> Vec<String> {
        values.iter().map(|value| (*value).to_string()).collect()
    }

    #[test]
    fn default_cli_action_launches_workspace() {
        assert_eq!(
            parse_cli_action(&args(&[])).unwrap(),
            CliAction::Launch(LaunchMode::Workspace)
        );
    }

    #[test]
    fn onboard_command_launches_onboarding_only() {
        assert_eq!(
            parse_cli_action(&args(&["onboard"])).unwrap(),
            CliAction::Launch(LaunchMode::OnboardingOnly)
        );
    }

    #[test]
    fn help_command_prints_usage() {
        assert_eq!(
            parse_cli_action(&args(&["--help"])).unwrap(),
            CliAction::PrintHelp
        );
    }

    #[test]
    fn unknown_command_is_rejected() {
        let error = parse_cli_action(&args(&["unknown"])).unwrap_err();

        assert!(error.contains("Unknown command: unknown"));
        assert!(error.contains("Usage: whatson [onboard]"));
    }

    #[test]
    fn onboarding_launch_mode_forwards_internal_app_flag() {
        assert_eq!(
            app_arguments(LaunchMode::OnboardingOnly),
            &["--onboarding-only"]
        );
        assert!(app_arguments(LaunchMode::Workspace).is_empty());
    }

    #[test]
    fn onboarding_mode_skips_direct_prebuilt_launch() {
        assert!(!should_try_prebuilt(LaunchMode::OnboardingOnly));
        assert!(should_try_prebuilt(LaunchMode::Workspace));
    }

    #[test]
    fn cmake_home_directory_is_extracted_from_cache_text() {
        let cache_text = "\
CMAKE_HOME_DIRECTORY:INTERNAL=/tmp/whatson
OTHER_KEY:STRING=value
";

        assert_eq!(
            extract_cmake_home_directory(cache_text),
            Some("/tmp/whatson")
        );
    }
}
