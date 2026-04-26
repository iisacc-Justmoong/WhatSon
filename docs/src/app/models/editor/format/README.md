# `src/app/models/editor/format`

## Responsibility
Owns editor-body inline formatting logic.

## Current Modules
- `ContentsTextFormatRenderer.*`
  Converts RAW `.wsnbody` inline tags into editor-surface HTML and exposes RAW mutation helpers for inline style
  commands.
- `ContentsTextHighlightRenderer.*`
  Keeps highlight alias and HTML style details out of the broader formatter implementation.
- `ContentsStructuredEditorFormattingController.qml`
  Coordinates selection-based inline formatting for the structured document flow while delegating RAW rewrite rules to
  the editor formatter.

## Boundary
- This directory owns body text formatting such as bold, italic, underline, strikethrough, and highlight.
- Paper and print geometry remain in `src/app/models/display/paper` and `src/app/models/display/paper/print`.
- The formatter may derive presentation from RAW source, but persisted writes still go through `.wsnote/.wsnbody`
  mutation paths first.
