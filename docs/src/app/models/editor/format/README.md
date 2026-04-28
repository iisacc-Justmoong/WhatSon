# `src/app/models/editor/format`

## Responsibility
Owns editor-body inline formatting logic.

## Current Modules
- `ContentsInlineStyleOverlayRenderer.*`
  Renders block-local inline style overlays for structured text delegates without exposing the full document renderer
  contract.
- `ContentsPlainTextSourceMutator.*`
  Owns plain-text source-span replacement and committed-URL canonicalization for ordinary typing paths.
- `ContentsRawInlineStyleMutationSupport.js`
  Builds direct RAW inline-style selection payloads by wrapping the resolved `.wsnbody` selection span with opening and
  closing tags.
- `ContentsTextFormatRenderer.*`
  Converts RAW `.wsnbody` document snapshots into editor-surface HTML, preview HTML, and normalized render payloads.
- `ContentsTextHighlightRenderer.*`
  Keeps highlight alias and HTML style details out of the broader formatter implementation.
- `ContentsStructuredEditorFormattingController.qml`
  Coordinates selection-based inline formatting for the structured document flow while delegating RAW rewrite rules to
  the pure inline-style mutation helper.

## Boundary
- This directory owns body text formatting such as bold, italic, underline, strikethrough, and highlight.
- Paper and print geometry remain in `src/app/models/display/paper` and `src/app/models/display/paper/print`.
- The formatter may derive presentation from RAW source, but persisted writes still go through `.wsnote/.wsnbody`
  mutation paths first.
