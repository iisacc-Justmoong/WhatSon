# `src/app/models/editor/EditorInputCommandFilter.hpp`

## Responsibility

Declares the editor-wide native key event filter used by the note editor surface.

## Contract

- Exposes `attachEditorInputOwner(...)` and `detachEditorInputOwner(...)` for QML to bind the public LVRS editor item,
  the editor wrapper, `ClipboardEditorPaste`, `InAppClipboardManager`, and `NoteEditorDocumentSession`.
- Owns event-filter lifetime for the editor item. `ClipboardEditorPaste` must remain a paste orchestration object and
  must not install editor item event filters.
- Consumes Backspace/Enter only when `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)` reports a handled
  semantic boundary operation.
- Consumes `Cmd/Ctrl+V` only after `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)` produces a non-native,
  handled paste result.
- Uses the wrapper's `editorCursorPosition` for image resource paste and sends paste as a collapsed insertion, so stale
  selections cannot turn image paste into a destructive replacement. Collapsed boundary commands also follow the current
  caret.
- Applies returned editor HTML through the editor wrapper's `replaceEditorDocumentText(...)` hook.

## 한국어

- 이 헤더는 에디터 전역 key event filter 객체의 QML 연결 API를 정의한다.
- 이벤트 필터 수명은 이 객체가 소유하지만 source boundary 결정은 `NoteEditorDocumentSession`에 남긴다.
- 이미지 resource paste는 wrapper의 `editorCursorPosition`을 기준으로 하고 selection을 접은 삽입으로 dispatch한다.
  selection이 없는 boundary 명령도 current caret을 따른다.
