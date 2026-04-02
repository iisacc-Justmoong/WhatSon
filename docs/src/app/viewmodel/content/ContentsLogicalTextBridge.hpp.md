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
- `normalizeLogicalText(...)` is the private normalization entrypoint used before line-offset
  generation so QML gutter queries consume plain-text-aligned offsets.
- `sourceOffsetForLogicalOffset(int)` is now part of the public QML bridge surface so editor interactions can convert
  RichText/plain-text selection offsets back into source-markup offsets before mutating the stored body text.
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
