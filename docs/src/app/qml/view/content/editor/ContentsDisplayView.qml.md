# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the unified desktop/mobile note editor layout host.

It composes:

- C++ editor display ViewModels under `src/app/viewmodel/editor/display`
- model-side display controllers under `src/app/models/editor/display`
- active display-surface policy assembly
- editor display controller and ViewModel assembly
- sibling host assembly for the surface, auxiliary rails, and root overlays

It no longer acts as the direct center-surface viewer for resource-backed hierarchy browsing.
`ContentViewLayout.qml` now mounts `ContentsResourceEditorView.qml` beside this host whenever the active list model is
not note-backed.

## Editing Model

- RAW `.wsnbody` remains the only write authority.
- `ContentsStructuredDocumentFlow.qml` is the canonical note host once a note session is bound.
- `ContentsDisplaySurfacePolicy` owns the active surface decision. A selected note requests the structured document
  surface; the legacy whole-note inline loader is no longer mounted by this host.
- The host no longer exposes a no-op inline editor proxy or pushes a RichText editing surface back into
  `ContentsInlineFormatEditor.qml`.
- Selection/mount, presentation, RAW mutation, and minimap/gutter command surfaces now pass through dedicated C++
  ViewModels under `src/app/viewmodel/editor/display`.
- Selected-note focus restoration now targets `ContentsStructuredDocumentFlow.qml` directly.
  The structured document host is the canonical note editor surface, so the root view no longer instantiates an
  extra active-surface adapter layer just to forward focus requests.
- QML runtime mechanics that need timers, connections, shortcuts, pointer handlers, or context menus live as
  model-side controllers under `src/app/models/editor/display`; they are not ViewModels.
- `ContentsDisplayView.qml` keeps compatibility controller functions only for collaborators that still call the display
  host API directly; those controllers delegate into the responsible C++ ViewModel or into narrow QML-side models such
  as `ContentsDisplayGeometrySnapshotModel.qml` and `ContentsDisplayViewportModel.qml`.
- Center-surface rendering now lives in `ContentsDisplaySurfaceHost.qml`, auxiliary gutter/minimap composition lives in
  `ContentsDisplayAuxiliaryRailHost.qml`, and root exception/conflict overlays live in
  `ContentsDisplayOverlayHost.qml`.
  `ContentsDisplayView.qml` keeps aliases to the mounted surface objects only because existing display controllers
  still need a single object contract for focus, geometry, and minimap repaint calls.
- `ContentsDisplayNoteBodyMountCoordinator` now owns the note-body mount contract between the selection bridge,
  the editor session, and the mounted document surface.
  The host stays in a pending mount state until either the selected note snapshot resolves for that same note id or
  the editor session binds directly to the selected note.
- If no note is selected, the host suppresses the body chrome and shows the centered `No document opened` placeholder
  plus a short instruction to pick a note from the list.
- If one snapshot refresh retry still leaves the selected note body unresolved or mismatched, the host now shows a
  reason-specific centered placeholder instead of collapsing every failure into `No document opened`.
  Current failure labels distinguish at least:
  - body snapshot mismatch against the current selection
  - unresolved note body after snapshot refresh
- Structured tag insertions target the parser-owned document flow rather than a hidden fallback editor surface.

## Debug Trace

- The host now emits explicit editor-creation trace turns for note selection, body-note resolution, editor-session
  binding, document mount pending/mounted/failure transitions, structured-host requests, and the active surface policy.
- Geometry/minimap snapshot planning and viewport line math are no longer implemented inline as one large block inside
  this file. The host keeps the public wrapper function names, but the heavy logic now lives in
  `src/app/models/editor/display/ContentsDisplayGeometrySnapshotModel.qml` and
  `src/app/models/editor/display/ContentsDisplayViewportModel.qml`.
- It now also emits note-selection flow plan traces end-to-end:
  - `selectionFlow.scheduleSelectionModelSync` when the host translates a note selection/body update into paired
    selection-sync and mount passes
  - `selectionFlow.pollPlan` / `selectionFlow.pollReconcileRequested` / `selectionFlow.pollSnapshotRefresh` for
    periodic same-note snapshot comparison
  - `selectionFlow.reconcilePlan` / `selectionFlow.reconcileRequested` for one-shot editor-entry reconciliation
  - `selectionFlow.selectionSyncPlan` / `selectionFlow.selectionSyncResult` for editor-session sync, cache reset,
    focus, and fallback-refresh handling emitted from `ContentsDisplaySelectionSyncCoordinator`
  - `selectionFlow.mountPlan` / `selectionFlow.mountResult` for the actual mount/snapshot-refresh/editor-bind outcome
- `scheduleSelectionModelSync(...)` now dispatches the same normalized option payload into both
  `ContentsDisplaySelectionSyncCoordinator` and `ContentsDisplayNoteBodyMountCoordinator`.
  The host also keeps those flush handlers on separate `Connections` blocks so a mount-plan signal cannot be lost by
  being attached to the wrong coordinator target.
- Snapshot poll and reconcile trace turns now call `traceFormatter.describeSelectionPlan(...)` directly.
  This avoids the host-local `TypeError` that previously aborted the periodic snapshot refresh path before the selected
  note body could be reconciled and remounted into the parser-backed document surface.
- The structured host path now logs `structuredDocumentFlowVisibleChanged` so it is visible exactly when the parsed
  document surface becomes the live editor surface.
- Host-owned creation logs now assign stable object names to the main editor-collaboration objects:
  - `contentsDisplaySelectionBridge`
  - `contentsDisplayNoteBodyMountCoordinator`
  - `contentsDisplayEditorSession`
  - `contentsDisplayStructuredDocumentFlow`
- Those names are mirrored in the QML trace payload so startup/runtime logs can reconstruct which object instance was
  created, destroyed, or selected as the active document surface during editor bootstrap.

## Presentation Refresh

- `ContentsEditorPresentationProjection` now exposes editor HTML, preview HTML, and logical text/line metadata.
- `ContentsDisplayView.qml` now copies projection logical-line metrics into explicit host state instead of relying on
  breakable QML property bindings.
- Projection `logicalLineCount` and `logicalLineStartOffsets` changes now schedule gutter refresh directly, so line
  numbers no longer wait for an unrelated cursor, scroll, or layout event before updating.
- `documentPresentationSourceText` now comes from the resolver's signal-backed property instead of a one-shot
  `Q_INVOKABLE` call.
  This keeps the structured document host and HTML projection synchronized when the selection bridge resolves a fresh
  same-note body snapshot after the initial mount attempt.
- The document-source resolver itself now lives under `src/app/models/editor/display`.
  QML still binds live selection/session state into it, but source arbitration is now treated as editor-domain C++
  logic rather than as a content-panel helper.
- The host now reads `documentSourcePlan` from the resolver's `Q_PROPERTY`, so the note-body mount coordinator is
  re-evaluated by QML as soon as the selected note body, editor session, or pending-save state changes.
- The host now also treats a bound editor session as matching the current selection only when the note id matches and,
  when available, `editorBoundNoteDirectoryPath == selectedNoteDirectoryPath`.
  This prevents the editor chrome from staying mounted on stale session state when the same `noteId` resolves to a
  different `.wsnote` package.
- The mount coordinator itself now binds directly to raw selection-bridge and editor-session state instead of feeding
  resolver-derived presentation choices back into the mount contract.
  Source arbitration remains the resolver's job; mount readiness now evaluates against the authoritative
  selection/session inputs without depending on a second layer of derived `documentSourcePlan` values.
- The host no longer publishes a separate `noteDocumentCommandSurfaceEnabled` readiness alias.
  Tag-management command surfaces now derive directly from `noteDocumentParseMounted` plus the active structured
  editor mode exposed through `ContentsEditorInputPolicyAdapter.qml`.
  Once RAW `.wsnbody` has been mounted into the parser-backed structured document host, shortcut and context-menu
  commands may write the next RAW mutation immediately without waiting for a separate editor-surface-ready flag.
- Ordinary window-level document shortcuts stand down while a native input session owns the keyboard.
  The tag-management shortcut surface is separate: inline style wrapping and structured tag insertion stay enabled while
  an editor is focused, but still stand down during structured or fallback IME/preedit composition.
- The host exposes `editorCustomTextInputEnabled: false`, `editorTagManagementInputEnabled: true`, and
  `noteDocumentTagManagementShortcutSurfaceEnabled` to make that policy explicit in QML and regression tests.
- Inline-format shortcut parsing treats either `Meta` or `Control` as the command accelerator so the RAW tag mutation
  path still runs on platforms that report native command chords with both modifier bits. `Alt`/Option remains excluded
  from these formatting commands.
- Editor selection context-menu invocation is now modeled as a shared pointer-trigger contract:
  desktop right-click and mobile touch/stylus long-press both route through
  `requestEditorSelectionContextMenuFromPointer(...)`.
  This keeps mobile access to the same tag-management menu without adding ordinary text key handlers, and the menu
  surface still stands down while a native composition/preedit session is active.
- The desktop path now primes that selection snapshot from a right-button `MouseArea` press before opening on click.
  This keeps a selected body range available for the menu even if the live `TextEdit` selection changes during the
  right-click release turn.
- Markdown list toggles are not exposed from this host. `Meta/Alt+Shift+7/8` list shortcuts and generic key-event
  forwarding are intentionally absent so ordinary text input remains native.
- Fallback-editor `inputMethodComposing`/`preeditText` change signals now only resume pending cursor restoration and
  resource-import surface restore after the native session has settled.
- The `ContentsDisplayDocumentSourceResolver` block keeps each upstream source property bound exactly once.
  Duplicate assignments in that block are a hard QML compile error, so the host now relies on one normalized binding
  per resolver input and the regression suite checks that those keys remain unique.
- Logical-line offset lookup and minimap viewport math now route through
  `ContentsDisplayViewportCoordinator` in `src/app/models/editor/display`.
  `ContentsDisplayView.qml` still owns cache invalidation and composition, but it no longer carries those binary
  searches and proportional-track calculations as local JavaScript helpers.
- Structured minimap snapshotting no longer starts from `cachedLogicalLineEntries`.
  The host now normalizes the parser-backed `ContentsStructuredDocumentFlow.normalizedBlocks()` stream into one minimap
  snapshot entry per block/tag, then asks `ContentsDisplayMinimapCoordinator` to build the synthetic silhouette rows.
  Text-like rows size themselves from block plain-text amount; visual blocks such as `<resource ... />` stay block-like.
- `commitDocumentPresentationRefresh()` refreshes only the HTML overlay/minimap projection; it no longer triggers a
  RichText surface reinjection step.
- Resource-bearing fallback notes still substitute `whatson-resource-block` placeholders into HTML, but that
  substitution now stays entirely inside the display pipeline.
- The non-print editor viewport now reserves the existing bottom accessibility inset at up to roughly half of the
  live editor surface height, so the last authored line can be pulled much higher above the shell bottom edge when the
  user scrolls to the document tail.
- Structured-flow, resource-render, and legacy-editor geometry changes now request gutter refresh through dedicated
  reasons instead of reusing the focused `line-structure` suppression path.
- Structured `cachedLogicalLineEntries` updates now split logical-metric change from geometry-only change.
  Even when line count and start offsets stay the same, resource/callout/agenda spacing or measured block-height
  changes still trigger a gutter refresh as soon as `contentY` / `gutterContentY` move.
  That structured logical-line cache still feeds gutter/logical-line coordination, but it is no longer the primary
  minimap row source.
- The line-number column now uses a dedicated `gutterBodyGap` token for its right inset instead of reusing the editor
  text column's `editorHorizontalInset`.
  This keeps note-body left padding unchanged while tightening the visual distance between gutter numbers and the first
  text glyph.
- The host no longer synthesizes an automatic `current` gutter marker for the cursor line.
  Current-line feedback remains limited to the line number's active color/font weight, and external `current` marker
  payloads are ignored by the marker bridge, so the editor does not draw an unexplained dot beside the first glyph of
  each text block.
- Note-entry gutter/minimap geometry is now also invalidated per selected note instead of trusting only line-count
  parity.
  Rapid note switches therefore clear stale incremental line caches immediately, queue a fresh structured layout-cache
  rebuild for the newly selected note, and only reuse minimap-derived line geometry once it has been regenerated for
  that same note id.
- Note-entry structured layout rebuilds now route through `scheduleStructuredDocumentOpenLayoutRefresh(...)` instead of
  calling the flow's single-pass cache refresh directly.
  That keeps the display host as the note-entry trigger while the structured document flow owns the post-open delegate
  measurement passes that make gutter coordinates reliable.
  The initial selected-note plan only enters that path after the resolved body id matches the selected note, so old-note
  geometry cannot satisfy the new note's pending gutter refresh.
  The selected-body text/resolved/loading handlers use the same open-layout path while a note-entry gutter refresh is
  pending, which keeps the first clean body snapshot and the first rendered block snapshot on the same measurement
  contract.
- Blur-driven immediate editor flush returns without saving while the live `TextEdit` is still composing or exposing
  preedit text. The host must not force a blur save after a fixed retry count because OS IME owns that unsettled input
  session.
- Editor shortcut, context-menu, and focus-restore gating now goes through `ContentsEditorInputPolicyAdapter.qml`.
  Ordinary text-edit focus requests are marked as `reason: "text-edit"` by the structured block delegate and are not
  replayed during a focused native-priority input session, which avoids reapplying focus/cursor between iOS delete
  repeat ticks.
- Selection delivery into `ContentsEditorSessionController` and `ContentsDisplayDocumentSourceResolver` now forwards
  `selectedNoteDirectoryPath` explicitly.
  QML no longer asks those collaborators to rediscover the mounted package from `noteId` alone after selection has
  already picked a concrete `.wsnote` directory.
- Page/print mode now also injects `paperPaletteEnabled` into both `ContentsEditorPresentationProjection` and
  `ContentsStructuredDocumentFlow.qml`, so the white paper surface cannot inherit dark-theme body white from either the
  whole-document HTML renderer or the structured block delegates.

## Resource Import

- Resource insertion still mutates RAW source first.
- Canonical RAW `<resource ... />` text generation now lives behind the `ContentsResourceTagTextGenerator` C++ bridge
  instead of a host-local JavaScript string builder.
- The same import path now applies resource insertion payloads through `applyDocumentSourceMutation(...)`, so
  structured-flow focus restore and persistence scheduling happen on the same host RAW mutation path as other editor
  rewrites.
- After import, the host restores the plain-text editor surface from projection logical text and recomputes the HTML
  overlay.
- Inline resource preview HTML is therefore presentation-only and cannot become editor write authority.
