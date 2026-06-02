# `src/app/qml/view/contents/TextEditor.qml`

`TextEditor.qml` is the LVRS editor surface kept for the contents layout.

## Current Contract

- The root item remains `LV.TextEditor`.
- `noteBodyFilePath` is still the public file-path alias, but the current layout passes an empty path.
- The surface is read-only when no file path is provided.
- The wrapper keeps view-local line metrics, cursor visibility, viewport scrolling, gutter alignment, and minimap data.

## Removed Responsibilities

The component no longer talks to an active note document session, clipboard paste bridge, native input command filter, RAW source projection, resource-frame reprojection, or persistence hook.

It must not install custom IME handling, Return/Enter overrides, paste shortcuts, inline-format shortcuts, or document-model adapters.
