# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility
Desktop content editor host.

## Structured Document Flow
- Structured notes now switch from the legacy overlay composition into `ContentsStructuredDocumentFlow.qml`.
- The flow renderer is activated whenever `ContentsStructuredBlockRenderer` reports a non-text block
  (`agenda`, `callout`, or `break`).
- In that mode, agenda/callout cards are rendered as document-owned blocks inside the scroll flow instead of being
  painted above/below a single `TextEdit`.
- Desktop structured shortcuts (`Cmd+Opt+T`, `Cmd+Opt+C`, `Cmd+Shift+H`) now dispatch through the view so they can
  target either the legacy single-editor path or the new structured flow path.
- Source persistence for block edits now runs through `applyDocumentSourceMutation(...)`, which updates the RAW body,
  marks local authority, optionally restores focus inside the reparsed block, and only forces a full legacy
  presentation rebuild when the note actually falls back out of structured-flow mode.
- While structured-flow mode is active, legacy agenda/callout overlay layers now receive empty models so hidden fallback
  delegates do not instantiate in parallel with the document-native block flow.
- Note-open reconcile is now scheduled through a deferred one-shot helper instead of running synchronously in the
  `selectedNoteIdChanged` turn.
- The host now tracks one pending note-entry reconcile id and waits for
  `selectionBridge.viewSessionSnapshotReconciled(...)` before marking that note's entry snapshot as compared.
- Timer-driven snapshot polling now treats reconcile as an asynchronous request; the eventual `selectedNoteBodyText`
  echo and reconcile-complete signal drive follow-up UI refresh work.
- While structured-flow mode is active, the hidden `ContentsInlineFormatEditor` is now effectively idled:
  - its `text` binding is cleared
  - its `enabled`/`visible` state is gated by `legacyInlineEditorActive`
  - content-height / line-count / cursor `Connections` detach from the hidden surface

## Legacy Surface
- The single `ContentsInlineFormatEditor` and overlay layers still exist as the fallback path for notes without any
  structured blocks.
