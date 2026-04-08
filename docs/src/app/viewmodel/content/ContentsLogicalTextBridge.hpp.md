# `src/app/viewmodel/content/ContentsLogicalTextBridge.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/content/ContentsLogicalTextBridge.hpp`
- Source kind: C++ header
- File name: `ContentsLogicalTextBridge.hpp`
- Approximate line count: 45

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

## Logical Text State

- The header now declares a dedicated normalized logical-text cache (`m_logicalText`) separate from
  raw editor source text (`m_text`).
- `logicalText` is now exposed to QML so selection logic can compare the live highlighted substring against the same
  plain-text projection used by gutter/minimap metrics.
- That logical projection is now intended to stay as close as possible to the note's own `.wsnbody` plain-text view,
  including raw markdown markers.
- `normalizeLogicalText(...)` is the private normalization entrypoint used before line-offset
  generation so QML gutter queries consume plain-text-aligned offsets.
- `sourceOffsetForLogicalOffset(int)` is now part of the public QML bridge surface so editor interactions can convert
  RichText/plain-text selection offsets back into source-markup offsets before mutating the stored body text.
- `logicalLengthForSourceText(QString)` is now also part of the public QML bridge surface so block-style mutations can
  measure rewritten source fragments in the same logical-text coordinate system used by the live editor selection.
- `logicalToSourceOffsets()` is now also exposed so `ContentsEditorTypingController.qml` can seed one whole-note
  offset table at presentation-commit time and then update that table incrementally during ordinary typing.
- `adoptIncrementalState(...)` is now also part of the public QML surface so the typing controller can push a freshly
  spliced `{source text, logical text, line starts, logical->source offsets}` bundle back into the bridge after each
  accepted typing mutation.
- The header also carries a logical-to-source offset cache (`m_logicalToSourceOffsets`) that stays synchronized with
  the normalized plain-text projection.
- The exposed offset contract remains `int`-based for QML callers, so implementation-side source bounds must be
  normalized before clamping against `QString::size()`.

### Classes and Structs
- `ContentsLogicalTextBridge`

### Enums
- None detected during scaffold generation.

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

## Regression Checks

- When the editor surface contains unordered-list markdown, `logicalText` must preserve the same raw marker prefix so
  QML selection offsets still match the source editor surface.
- When QML rewrites one source line in isolation, `logicalLengthForSourceText(...)` must report the rendered logical
  character count for that fragment rather than the raw source-token count.
- When the host editor commits a new presentation snapshot, `logicalToSourceOffsets()` must expose an array with
  `logicalText.length + 1` entries so the typing controller can reseed its incremental mapping state without walking
  one QML-to-C++ call per character.
- When QML pushes `adoptIncrementalState(...)`, the bridge must accept line-start offsets and logical/source offset
  tables that already reflect the latest live keystroke, instead of rebuilding those tables from the whole note again.
