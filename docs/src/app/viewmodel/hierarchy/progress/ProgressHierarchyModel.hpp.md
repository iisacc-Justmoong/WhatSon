# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyModel.hpp`

## Responsibility

This header defines the flat list model used by the progress sidebar.

## Icon Mapping

- `progressHierarchyIconName(...)` normalizes labels by removing spaces, hyphens, and underscores
  before matching them.
- Known labels such as `Ready`, `Pending`, `InProgress`, `Done`, `Archived`, and legacy aliases map
  to stable LVRS icon names.
- Unknown labels intentionally fall back to an empty icon name so custom progress states can still
  render without forcing an incorrect glyph.
