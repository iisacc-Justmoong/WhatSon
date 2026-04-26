# `src/app/models/editor/resource/ContentsResourceDropPayloadParser.qml`

## Role
`ContentsResourceDropPayloadParser.qml` owns drag-and-drop payload normalization for resource import.

## Responsibilities
- Reads `drop.urls` when the platform exposes a native URL list directly.
- Falls back to string payloads such as `drop.text`, `text/uri-list`, `text/plain`, and platform file-url MIME values.
- Returns one normalized URL list without depending on editor/session state.
