# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

Provides the note-backed center editor surface.

## Current Contract

- Reads the active note identity and RAW body text from the supplied `noteListModel`.
- Synchronizes that snapshot into `ContentsEditorSessionController`.
- Binds `ContentsEditorPresentationProjection.sourceText` to the session RAW text.
- Binds LVRS metric tokens and projection line counts into `ContentsGutterLayoutMetrics` and
  `ContentsMinimapLayoutMetrics`, then consumes their resolved layout values.
- Binds editor projection logical offsets and the live `ContentsStructuredDocumentFlow.qml` geometry hooks into
  `ContentsGutterLineNumberGeometry`, then passes the resolved line-number entries into `contents/Gutter.qml`.
- Binds live cursor position, current RAW text, saved `.wsnbody` RAW text, and line-number entries into
  `ContentsGutterMarkerGeometry`, then passes the resolved marker entries into `contents/Gutter.qml`.
- Forwards `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks` into
  `ContentsStructuredDocumentFlow.qml`.
- Mounts the actual editor chrome as an `LV.HStack`: existing `contents/Gutter.qml` on the left,
  `ContentsStructuredDocumentFlow.qml` in the center, and existing `contents/Minimap.qml` on the right.
- Commits user edits through `ContentsEditorSessionController.commitRawEditorTextMutation(...)` first, then asks the
  active hierarchy controller to persist through `saveCurrentBodyText(...)` when that capability is available.

## Pipeline

The live route is:

1. `.wsnbody` RAW text enters the editor session.
2. The presentation projection reparses and renders that RAW text.
3. `ContentsHtmlBlockRenderPipeline` validates the HTML projection, runs `iiHtmlBlock`, and publishes block metadata.
4. `ContentsGutterLayoutMetrics` and `ContentsMinimapLayoutMetrics` resolve chrome widths and row counts from tokens
   plus `logicalLineCount`.
5. `ContentsGutterLineNumberGeometry` samples the live `LV.TextEditor` line rectangles and maps them into gutter
   coordinates.
6. `ContentsGutterMarkerGeometry` projects the cursor line and unsaved RAW-source lines onto the same gutter
   coordinates.
7. `ContentsDisplayView.qml` places gutter, editor document slot, and minimap in one `LV.HStack`.
8. `ContentsStructuredDocumentFlow.qml` displays the final RichText projection and keeps the plain `LV.TextEditor` buffer as
   the edit source inside the center slot.

The QML host does not parse XML, does not derive block boundaries from DOM or RichText output, and no longer owns
gutter/minimap metric arithmetic, gutter line-number y-coordinate projection, or gutter marker state calculation.
