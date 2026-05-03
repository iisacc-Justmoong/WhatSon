# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

Provides the note-backed center editor surface.

## Current Contract

- Reads the active note identity and RAW body text from the supplied `noteListModel`.
- Synchronizes that snapshot into `ContentsEditorSessionController`.
- Binds `ContentsEditorPresentationProjection.sourceText` to the session RAW text.
- Forwards `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks` into
  `ContentsStructuredDocumentFlow.qml`.
- Commits user edits through `ContentsEditorSessionController.commitRawEditorTextMutation(...)` first, then asks the
  active hierarchy controller to persist through `saveCurrentBodyText(...)` when that capability is available.

## Pipeline

The live route is:

1. `.wsnbody` RAW text enters the editor session.
2. The presentation projection reparses and renders that RAW text.
3. `ContentsHtmlBlockRenderPipeline` validates the HTML projection, runs `iiHtmlBlock`, and publishes block metadata.
4. `ContentsStructuredDocumentFlow.qml` displays the final RichText projection and keeps the plain `TextEdit` buffer as
   the edit source.

The QML host does not parse XML and does not derive block boundaries from DOM or RichText output.
