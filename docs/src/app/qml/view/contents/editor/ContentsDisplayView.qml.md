# `src/app/qml/view/contents/editor/ContentsDisplayView.qml`

## Responsibility

Provides the note-backed center editor surface.

## Current Contract

- Reads the active note identity and RAW body text from the supplied global `noteActiveState` when available, falling
  back to the supplied `noteListModel` only for legacy host paths.
- Registers its visible `ContentsEditorSessionController` with `noteActiveState`; the global active-note object then
  synchronizes that snapshot into the session on the same turn as note selection.
- Binds `ContentsEditorPresentationProjection.sourceText` to the session RAW text.
- Re-parses the same RAW snapshot through `ContentsStructuredBlockRenderer`, feeds that parser-owned block stream into
  `ContentsBodyResourceRenderer`, and lets `ContentsInlineResourcePresentationController` replace
  `whatson-resource-block` iiHtmlBlock placeholders with actual framed renderable resource HTML.
- Passes the current structured editor text-column width into the inline resource renderer so every resource frame fills
  100% of the note body column instead of keeping the historical fixed `480px` display width.
- Forwards the resource-resolved `editorSurfaceHtml`, plus `logicalText`, `htmlTokens`, and `normalizedHtmlBlocks`,
  into `ContentsStructuredDocumentFlow.qml`.
- Converts the live RAW cursor offset into a logical display cursor offset through
  `ContentsEditorPresentationProjection.logicalLengthForSourceText(...)` before handing it to the structured flow.
  The inline editor uses that value only for visible caret projection above the RichText overlay.
- Keeps live typing and committed terminator input on the same presentation path: `sourceTextEdited(...)` mutates the
  RAW editor session, `ContentsEditorPresentationProjection` rerenders from that RAW text, and the inline editor paints
  the result through its read-only `TextEdit.RichText` overlay. Resource-backed overlays are pinned while this live
  loop is in flight, so a transient render turn cannot reveal the source `<resource ... />` tag.
- Relies on `ContentsStructuredDocumentFlow.qml` for the bottom-empty-area accessibility hit target: clicking below the
  rendered body inside the center editor behaves like clicking the RAW body end, without changing the persistence path.
- Wraps `ContentsStructuredDocumentFlow.qml` in a dedicated editor document `Flickable`. The viewport content height is
  driven by the flow's rendered `editorContentHeight`; long notes stay readable through the center document viewport.
- Resets that viewport only when the mounted note identity changes. RAW body refreshes from persistence keep the current
  scroll position, so saving or live synchronization cannot snap a long document back to the top.
- Mounts the editor chrome as an `LV.HStack`: the scrollable `ContentsStructuredDocumentFlow.qml` viewport in the
  center and existing `view/contents/Minimap.qml` on the right.
- Commits user edits through `ContentsEditorSessionController.commitRawEditorTextMutation(...)` first, then asks the
  active hierarchy controller to persist through `saveCurrentBodyText(...)` when that capability is available.

## Pipeline

The live route is:

1. `noteActiveState` receives the committed active note id, package path, and `.wsnbody` RAW body snapshot from the
   active hierarchy's note-list model.
2. `noteActiveState` synchronizes the attached editor session immediately through
   `ContentsEditorSessionController.requestSyncEditorTextFromSelection(...)`.
3. `.wsnbody` RAW text enters the editor session.
4. The presentation projection reparses and renders that RAW text.
5. `ContentsHtmlBlockRenderPipeline` validates the HTML projection, runs `iiHtmlBlock`, and publishes block metadata.
6. `ContentsBodyResourceRenderer` resolves parser-owned `resource` blocks against the selected note directory, then
   `ContentsInlineResourcePresentationController` replaces matching iiHtmlBlock placeholders with the framed inline
   image HTML used by the historical Figma `292:50` resource presentation when the payload is renderable. The frame
   width is resolved from the live editor text column before HTML is published.
7. `ContentsMinimapLayoutMetrics` uses projection line counts for the minimap surface.
8. `ContentsDisplayView.qml` places a scrollable editor document viewport and minimap in one `LV.HStack`.
9. `ContentsStructuredDocumentFlow.qml` displays the final RichText projection through the inline editor's read-only
   `TextEdit.RichText` overlay and keeps the plain `LV.TextEditor` buffer as the edit source inside the center slot.
10. The inline editor paints a projected blinking cursor above the overlay from logical visible-text geometry while the
    native `LV.TextEditor` keeps the authoritative cursor and IME state. The native cursor delegate is hidden whenever
    the rendered overlay is visible, so the user sees one WYSIWYG caret instead of a second RAW-surface caret.
11. Bottom-empty-area clicks inside that center slot focus the same live editor and place the cursor at the RAW source
    end. No synthetic text mutation is created by this accessibility path.

The QML host does not parse XML, does not derive block boundaries from DOM or RichText output, and no longer owns
minimap metric arithmetic.
