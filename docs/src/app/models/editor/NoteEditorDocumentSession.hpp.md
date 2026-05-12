# `src/app/models/editor/NoteEditorDocumentSession.hpp`

## Responsibility

Declares the active note editor document session object.

## Contract

- Exposes `editorFilePath`, the editor HTML session file consumed by `LV.TextEditor.filePath`.
- Tracks the active note id and note directory path from `NoteActiveStateTracker`.
- Exposes `parsedLineCount` as canonical RAW source line metadata. The value is derived before source is projected into
  editor HTML or after synced editor HTML is converted back to source; QML uses this value as the gutter delegate
  count instead of LVRS rendered wrap-line count.
- Exposes `loading`, `readOnly`, and `lastError` so QML can keep the native editor surface guarded while C++ loads or
  clears a note.
- Provides `persistEditorFile(path)` for the LVRS `syncFinished` hook.
- Provides `insertImportedResourcesIntoSource(...)`, which converts imported resource metadata into canonical
  standalone RAW `<resource ... />` source insertion results plus an editor HTML projection for clipboard/drop flows.
- Provides `insertFormatTagIntoSource(...)`, which applies a static editor format tag such as `bold`, `italic`,
  `underline`, `strikethrough`, `highlight`, or `break` through `SetTag`, then returns both canonical RAW source and an editor HTML projection for the
  live LVRS surface.

## Guardrails

- The editor file path must not point at the raw `.wsnbody` XML document.
- LVRS `TextEditor` is a rich-text surface, so loaded source must be projected to editor HTML with explicit line breaks
  before it reaches `filePath`.
- `.wsnbody` parsing and serialization stay in C++ note/editor model code, not QML.
- Format tag allow-list and mutation policy stay in `SetTag`; QML may only pass the requested tag name and current
  cursor/selection metadata into this session boundary.

## 한국어

- 이 객체는 선택된 노트를 LVRS `TextEditor`에 연결하기 위한 C++ 세션 경계다.
- `editorFilePath`는 `.wsnbody` 원문이 아니라 editor HTML session file이어야 한다.
- `parsedLineCount`는 canonical RAW source line metadata이며 QML이 직접 파일을 읽거나 파싱하지 않게 한다.
  거터의 실제 row 개수는 이 값만 따르며, LVRS rendered wrap-line count를 따르지 않는다.
- LVRS가 session file 저장을 끝내면 `persistEditorFile(...)`이 다시 `.wsnbody` 저장 경로로 넘긴다.
- clipboard/drop resource metadata를 노트 source에 넣을 때는 `insertImportedResourcesIntoSource(...)`가
  canonical `<resource ... />` 삽입 결과와 editor HTML projection을 함께 계산한다.
- `bold`, `italic`, `underline`, `strikethrough`, `highlight`, `break` 같은 포맷 태그는 `insertFormatTagIntoSource(...)`가 `SetTag`를 통해
  RAW source와 editor HTML projection을 함께 계산한다.
