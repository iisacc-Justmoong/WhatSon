# `src/app/models/content/ContentsLogicalTextBridge.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/models/content/ContentsLogicalTextBridge.hpp`
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
  plain-text projection used by minimap metrics.
- That logical projection is now intended to stay as close as possible to the note's own `.wsnbody` plain-text view,
  including raw markdown markers.
- `normalizeLogicalText(...)` is the private normalization entrypoint used before source-offset generation so editor
  queries consume plain-text-aligned offsets.
- `sourceOffsetForLogicalOffset(int)` is now part of the public QML bridge surface so editor interactions can convert
  RichText/plain-text selection offsets back into source-markup offsets before mutating the stored body text.
- `logicalOffsetForSourceOffset(int)` is also part of the public bridge surface so the editor can project the live RAW
  cursor onto visible logical text without slicing the source through possibly incomplete inline tags.
- `logicalLengthForSourceText(QString)` is now also part of the public QML bridge surface so block-style mutations can
  measure rewritten source fragments in the same logical-text coordinate system used by the live editor selection.
- The header also carries a logical-to-source offset cache (`m_logicalToSourceOffsets`) that stays synchronized with
  the normalized plain-text projection.
- `logicalOffsetForSourceOffsetWithAffinity(...)` and `sourceOffsetForVisibleLogicalOffset(...)` expose model-owned
  coordinate queries so QML does not receive or iterate the whole-note offset table.
- `logicalToSourceOffsetsChanged()` is an internal model invalidation signal consumed by higher-level C++ projection
  objects. It is not a QML offset-table binding contract.
- The exposed offset contract remains `int`-based for QML callers, so implementation-side source bounds must be
  normalized before clamping against `QString::size()`.
- Logical break mapping now treats canonical divider tokens (`</break>`) the same as `<br>`/`<hr>` so QML offset
  conversions keep divider rows addressable in one logical step.
- Resource tags now reserve one logical text line for source-offset mapping while the structured document flow owns
  their rendered frame geometry.

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
- When the host editor commits a new presentation snapshot, the internal logical-to-source offset cache must contain
  `logicalText.length + 1` entries so model coordinate queries map visible editor offsets back into RAW source.
- When source text includes `</break>`, the exported logical/source offset table must still include one logical
  position for that divider token.
- When source text includes `<resource ... />`, the exported logical/source offset table must include one atomic
  U+FFFC resource placeholder so selection and edit deltas map to the resource tag as one element.
- When the RAW cursor is inside an inline formatting tag token, `logicalOffsetForSourceOffset(...)` must return the
  visible logical boundary next to that zero-width tag token rather than counting the partial tag text.
