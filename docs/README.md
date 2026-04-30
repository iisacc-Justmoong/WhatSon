# Documentation Mirror

This directory mirrors the maintainable `src` tree under `docs/src/...`.

The mirror has two goals.
- Preserve a one-directory/one-file correspondence with the live codebase.
- Allow the repository to be documented incrementally without inventing a separate documentation taxonomy.

## Current State
- The full `src` tree has been mirrored into `docs/src/...`.
- Every maintained source directory has a `README.md`.
- Every maintained source file has a sibling `.md` document.
- A first detailed pass now exists for the core startup, model-domain runtime contract, hierarchy bridge, and sidebar composition areas.
- The remaining files still contain scaffold text and must be upgraded in later passes.

## Generation Rules
- Mirrors stable source directories under `docs/src/...`.
- Excludes hidden files, `.DS_Store`, and generated Rust build output under `src/cli/target`.
- Uses `README.md` as the directory entry point.
- Uses `<source-file-name>.md` for file-level documents.

## Recommended Reading Order
1. `docs/src/app/README.md`
2. `docs/src/app/main.cpp.md`
3. `docs/src/app/qml/Main.qml.md`
4. `docs/src/app/models/file/hierarchy/README.md`
5. `docs/src/app/models/panel/README.md`
6. `docs/src/app/qml/view/panels/sidebar/README.md`

## Authoring Rule
- Keep documentation grounded in the live implementation.
- Prefer naming concrete classes, signals, QML objects, and ownership paths over vague summaries.
- When architecture changes, update both the directory README and the affected file documents.
