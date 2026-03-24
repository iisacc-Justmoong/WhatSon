# `src/cli/src/main.rs`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/cli/src/main.rs`
- Source kind: Rust source
- File name: `main.rs`
- Approximate line count: 325

## Rust Surface Snapshot
### Functions
- `main`
- `usage_text`
- `parse_cli_action`
- `should_try_prebuilt`
- `discover_root`
- `is_project_root`
- `app_arguments`
- `try_launch_prebuilt`
- `build_and_launch`
- `pick_cmake_build_dir`
- `build_cache_exists`
- `build_cache_matches_root`
- `extract_cmake_home_directory`
- `paths_match`
- `clear_stale_cmake_cache`
- `remove_file_if_exists`
- `remove_dir_if_exists`
- `args`
- `default_cli_action_launches_workspace`
- `onboard_command_launches_onboarding_only`

### Structs
- None detected during scaffold generation.

### Enums
- `LaunchMode`
- `CliAction`

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
