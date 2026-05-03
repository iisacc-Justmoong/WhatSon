# `src/app/qml/view/contents/editor/ContentsDisplayView.qml`

## Responsibility

Provides the note-backed center editor surface.

## Current Contract

- Reads the active note identity and RAW body text from the supplied global `noteActiveState` when available, falling
  back to the supplied `noteListModel` only for legacy host paths.
- Registers its visible `ContentsEditorSessionController` with `noteActiveState`; the global active-note object then
  synchronizes that snapshot into the session on the same turn as note selection.
- Binds `ContentsEditorPresentationProjection.sourceText` to the session RAW text.
- Binds LVRS metric tokens and projection line counts into `ContentsGutterLayoutMetrics` and
  `ContentsMinimapLayoutMetrics`, then consumes their resolved layout values.
- Binds RAW source text, presentation-owned flattened display blocks, parser-owned logical document blocks, rendered
  resource metadata, and rendered content height into
  `ContentsGutterLineNumberGeometry`, then passes the resolved line-number entries into `view/contents/Gutter.qml`.
  Line-number y positions are block-stream based, with a zero top inset so line 1 starts flush with the editor body.
- Refreshes gutter line-number and marker geometry whenever note active state, editor-session text/binding state,
  presentation line count, parser block output, rendered resources, viewport size/scroll, or rendered content height
  changes. Gutter projection is intentionally repeatable; the editor host does not assume that one calculation made at
  note-open time remains valid for the rest of the note session.
- Passes `editorPresentationProjection.normalizedHtmlBlocks`, parser-owned
  `structuredBlockRenderer.renderedDocumentBlocks`, and resolved `bodyResourceRenderer.renderedResources` into the
  gutter geometry. The flattened display blocks identify the actual visible block stream, the structured blocks preserve
  logical grouping and source spans, and the gutter model merges complementary ranges from both streams during refresh.
  Resource metadata lets visible image-frame resources receive the additional body height they occupy in the editor
  surface while unresolved placeholders remain normal text-height rows.
- Binds live cursor position, current RAW text, saved `.wsnbody` RAW text, and line-number entries into
  `ContentsGutterMarkerGeometry`, then passes the resolved marker entries into `view/contents/Gutter.qml`.
- Re-parses the same RAW snapshot through `ContentsStructuredBlockRenderer`, feeds that parser-owned block stream into
  `ContentsBodyResourceRenderer`, and lets `ContentsInlineResourcePresentationController` replace
  `whatson-resource-block` iiHtmlBlock placeholders with actual framed renderable resource HTML.
- Forwards the resource-resolved `editorSurfaceHtml`, plus `logicalText`, `htmlTokens`, and `normalizedHtmlBlocks`,
  into `ContentsStructuredDocumentFlow.qml`.
- Converts the live RAW cursor offset into a logical display cursor offset through
  `ContentsEditorPresentationProjection.logicalLengthForSourceText(...)` before handing it to the structured flow.
  The inline editor uses that value only for visible caret projection above the RichText overlay.
- Keeps live typing and committed terminator input on the same presentation path: `sourceTextEdited(...)` mutates the
  RAW editor session, `ContentsEditorPresentationProjection` rerenders from that RAW text, and the inline editor paints
  the result through its read-only `TextEdit.RichText` overlay.
- Relies on `ContentsStructuredDocumentFlow.qml` for the bottom-empty-area accessibility hit target: clicking below the
  rendered body inside the center editor behaves like clicking the RAW body end, without changing the persistence path.
- Wraps `ContentsStructuredDocumentFlow.qml` in a dedicated editor document `Flickable`. The viewport content height is
  driven by the flow's rendered `editorContentHeight`, and content-y changes refresh gutter geometry so long notes stay
  readable without letting line numbers drift away from visible document rows.
- Resets that viewport only when the mounted note identity changes. RAW body refreshes from persistence keep the current
  scroll position, so saving or live synchronization cannot snap a long document back to the top.
- Mounts the actual editor chrome as an `LV.HStack`: existing transparent `view/contents/Gutter.qml` on the left,
  the scrollable `ContentsStructuredDocumentFlow.qml` viewport in the center, and existing `view/contents/Minimap.qml`
  on the right.
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
   image HTML used by the historical Figma `292:50` resource presentation when the payload is renderable.
7. `ContentsGutterLayoutMetrics` and `ContentsMinimapLayoutMetrics` resolve chrome widths and row counts from tokens
   plus `logicalLineCount`.
8. `ContentsGutterLineNumberGeometry` reads both the flattened visible block stream and the logical structured block
   stream to build line-number y positions, merging missing source ranges from the secondary stream when a refresh only
   has a partial projection. The actual rendered editor content height is reconciled against resolved image-resource
   entries, with parser-owned resource blocks as a fallback, so visible resource rows reserve their real displayed
   height in the gutter and later line numbers are shifted by the same amount as the editor body.
9. Note active-state changes, session synchronization, live RAW text edits, parser background refresh, resource
   renderer updates, projection line-count changes, and viewport movement can all request another gutter refresh.
   Recalculation is a normal part of the editor state loop, not a one-time layout step.
10. `ContentsGutterMarkerGeometry` projects the cursor line and unsaved RAW-source lines onto the same gutter
   coordinates.
11. `ContentsDisplayView.qml` places gutter, a scrollable editor document viewport, and minimap in one `LV.HStack`.
12. `ContentsStructuredDocumentFlow.qml` displays the final RichText projection through the inline editor's read-only
   `TextEdit.RichText` overlay and keeps the plain `LV.TextEditor` buffer as the edit source inside the center slot.
13. The inline editor paints a projected blinking cursor above the overlay from logical visible-text geometry while the
    native `LV.TextEditor` keeps the authoritative cursor and IME state. The native cursor delegate is hidden whenever
    the rendered overlay is visible, so the user sees one WYSIWYG caret instead of a second RAW-surface caret.
14. Bottom-empty-area clicks inside that center slot focus the same live editor and place the cursor at the RAW source
    end. No synthetic text mutation is created by this accessibility path.

The QML host does not parse XML, does not derive block boundaries from DOM or RichText output, and no longer owns
gutter/minimap metric arithmetic, gutter line-number y-coordinate projection, or gutter marker state calculation.
