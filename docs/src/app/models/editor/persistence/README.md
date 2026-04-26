# `src/app/models/editor/persistence`

## Responsibility
Reserved for editor-owned persistence orchestration that still preserves RAW `.wsnote/.wsnbody` as the only write
authority.

## Boundary
- File storage primitives remain under `src/app/models/file` until an editor-specific persistence coordinator is
  needed.
- Any future helper here must mutate RAW source first, then let parser-derived projections update the UI.
