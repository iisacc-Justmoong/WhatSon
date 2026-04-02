# `src/app/viewmodel/content/ContentsLogicalTextBridge.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/content/ContentsLogicalTextBridge.cpp`
- Source kind: C++ implementation
- File name: `ContentsLogicalTextBridge.cpp`
- Approximate line count: 127

## Runtime Line-Metric Contract

- The bridge now normalizes incoming editor source through `QTextDocument::setHtml(...)` +
  `toPlainText()` before deriving logical offsets.
- Paragraph and line separators are folded into `\n`, so logical line starts stay stable across
  rich-text paragraph blocks (`<p>`, `<br>`, inline span tags).
- `logicalLineCharacterCountAt(...)` uses normalized plain-text length (`m_logicalText`) instead of
  raw rich-text source length. This prevents gutter/minimap geometry drift when the source contains
  markup tokens that do not map 1:1 to cursor offsets.
- The implementation now also emits a QML-visible `logicalText` projection, giving selection-formatting code access to
  the exact plain-text string that the offset table was built from.
- The implementation now also builds a logical-text-to-source offset table:
  - inline tags are treated as zero-width source tokens
  - `<br>` counts as one logical line-break character
  - common HTML entities (`&lt;`, `&amp;`, `&#39;`, etc.) collapse to one logical character
- `sourceOffsetForLogicalOffset(...)` exposes that table to QML so selection/context-menu formatting can mutate the
  correct source slice even when the editor cursor operates on rendered plain-text offsets.
- Source-offset clamping is now normalized through an explicit `int`-bounded QString size helper before returning to
  QML, which avoids libc++ `std::clamp` template mismatches between `int` and `qsizetype` on Apple toolchains.

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- None detected during scaffold generation.

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
