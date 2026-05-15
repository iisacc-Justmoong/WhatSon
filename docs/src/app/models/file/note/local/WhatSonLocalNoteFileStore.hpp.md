# `src/app/models/file/note/local/WhatSonLocalNoteFileStore.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/models/file/note/local/WhatSonLocalNoteFileStore.hpp`
- Source kind: C++ header
- File name: `WhatSonLocalNoteFileStore.hpp`
- Approximate line count: 73

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- `WhatSonLocalNoteFileStore`
- `ReadRequest`
- `CreateRequest`
- `UpdateRequest`
- `UpdateResult`
- `DeleteRequest`

## UpdateRequest Contract

- `UpdateRequest` carries the local note document plus persistence toggles for header/body writes.
- `touchLastModified` updates the header timestamp during the write transaction.
- `incrementModifiedCount` opts a transaction into persisted `fileStat.modifiedCount` advancement, but the counter only
  advances when the transaction also has a serialized `.wsnhead` / `.wsnbody` payload diff that will be captured in
  `.wsnversion`.
- `refreshIncomingBacklinkStatistics` decides whether the update also pays the hub-wide `.wsnbody` scan needed to
  recompute `backlinkByCount` for the edited note.
- `refreshAffectedBacklinkTargets` decides whether changed backlink targets should have their own
  `backlinkByCount` refreshed immediately in the same transaction.
- `baseLastModifiedAt` carries the timestamp observed when the editor pulled the note.
- `incomingLastModifiedAt` can pin the editor-side save timestamp; when omitted, the store uses the current note
  timestamp for a body save.
- `resolveTimestampConflicts` enables the `file/conflict/WhatSonTimestampConflictResolver` policy. When the filesystem
  note advanced after `baseLastModifiedAt`, the store keeps the newer body by timestamp instead of blindly overwriting
  the other device's edit.
- The intended split is:
  - explicit metadata / structural note writes: keep `incrementModifiedCount == true`
  - changed editor body saves: keep `incrementModifiedCount == true`
  - unchanged editor body snapshots and timestamp-only header rewrites: rely on the store's versioned-payload diff
    comparison so the counter is not inflated
  - editor hot-path body writes that must stay latency-sensitive: set both backlink-refresh flags to `false` and let a
    higher-level owner trigger that hub scan later
- `UpdateResult` reports whether the transaction pushed a timestamped version diff to `.wsnversion`, the version file
  path that changed, the generated UTC timestamps copied from the header/body diff segments, and whether a timestamp
  conflict was resolved by choosing `incoming` or `filesystem`. Higher-level owners use this result to acknowledge the
  write as a local filesystem mutation for hub sync.

## Tests

- Regression checklist:
  - callers must be able to update `lastModifiedAt` without also incrementing `modifiedCount`
  - direct editor body persistence must advance `modifiedCount` when the RAW body changes
  - existing update callers that do not override `incrementModifiedCount` must keep commit-counter behavior when their
    transaction writes a real versioned header/body diff
  - timestamp-only header saves must not advance `modifiedCount` or append an empty `.wsnversion` snapshot
  - when `incrementModifiedCount` advances by exactly one, the same transaction must also emit a
    `.wsnversion` snapshot labeled as `commit:<modifiedCount>` via `file/diff/WhatSonLocalNoteVersionStore`
  - when that snapshot is written, `UpdateResult.versionDiffPushedToFilesystem` must be true and must carry the
    persisted `.wsnversion` path plus the generated diff timestamps
  - callers that disable backlink refresh must still get local body-derived counters (`lineCount`, `backlinkToCount`,
    `includedResourceCount`, etc.) rewritten into `.wsnhead`
  - stale editor saves with an older incoming timestamp must keep a newer filesystem body
  - editor saves with a newer incoming timestamp may supersede a filesystem body that changed after the editor pull

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
