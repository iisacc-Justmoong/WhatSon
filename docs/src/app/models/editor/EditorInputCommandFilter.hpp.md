# `src/app/models/editor/EditorInputCommandFilter.hpp`

## Responsibility

Declares the editor-wide native key event filter used by the note editor surface.

## Contract

- Exposes `attachEditorInputOwner(...)` and `detachEditorInputOwner(...)` for QML to bind the public LVRS editor item,
  the editor wrapper, `ClipboardEditorPaste`, `InAppClipboardManager`, and `NoteEditorDocumentSession`.
- Owns event-filter lifetime for the editor item. `ClipboardEditorPaste` must remain a paste orchestration object and
  must not install editor item event filters.
  `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)` reports a handled semantic boundary operation.
- Consumes `Cmd/Ctrl+V` only after `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)` produces a non-native,
  handled paste result.
- Applies returned editor HTML through the editor wrapper's `replaceEditorDocumentText(...)` hook.

## 한국어

- 이 헤더는 에디터 전역 key event filter 객체의 QML 연결 API를 정의한다.
  `NoteEditorDocumentSession`에 남긴다.
