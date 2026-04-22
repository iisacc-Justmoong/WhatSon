# `ContentsDisplayContextMenuCoordinator.cpp`

- Normalizes structured-selection snapshots and gates context-menu plans on finite numeric offsets.
- Uses explicit `QVariant::toDouble(&ok)` parsing so invalid snapshot payloads fail closed instead of compiling with an
  invalid overload.
