# `src/app/models/editor/EditorInputCommandFilter.hpp`

## Responsibility

Declares the editor-wide native key event filter used by the note editor surface.

## Contract

- Exposes `attachEditorInputOwner(...)` and `detachEditorInputOwner(...)` for QML to bind the public LVRS editor item,
  the editor wrapper, `ClipboardEditorPaste`, `InAppClipboardManager`, and `NoteEditorDocumentSession`.
- Owns event-filter lifetime for the editor item. `ClipboardEditorPaste` must remain a paste orchestration object and
  must not install editor item event filters.
- Consumes Backspace only when `NoteEditorDocumentSession.handleCalloutBoundaryKeyInSource(...)` reports a handled
  callout boundary operation. Consumes Return/Enter only when `NoteEditorDocumentSession.handleStyleBoundaryKeyInSource(...)`
  reports a handled style-boundary exit.
- Consumes `Cmd/Ctrl+V` only after `ClipboardEditorPaste.pasteImageResourceIntoEditor(...)` produces a non-native,
  handled paste result.
- Uses the wrapper's public selection metadata for image resource paste, preserving real selections instead of forcing a
  collapsed insertion. Collapsed callout Backspace commands follow the public LVRS `cursorPosition`.
- Applies returned editor HTML through the editor wrapper's `replaceEditorDocumentText(...)` hook.

## 한국어

- 이 헤더는 에디터 전역 key event filter 객체의 QML 연결 API를 정의한다.
- 이벤트 필터 수명은 이 객체가 소유하지만 callout/style source boundary 결정은 `NoteEditorDocumentSession`에 남긴다.
- 이미지 resource paste는 wrapper의 공개 selection metadata를 그대로 전달한다. Return/Enter는 style boundary exit가
  확인된 경우에만 선점한다.
