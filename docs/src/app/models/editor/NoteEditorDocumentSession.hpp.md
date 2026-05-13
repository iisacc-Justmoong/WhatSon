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
- Provides `insertImportedResourcesIntoSource(...)`, which receives resource package metadata already persisted by
  `InAppClipboard`, inserts canonical RAW `<resource ... />` calls at the current editor cursor/selection, and returns
  an editor HTML projection for the live LVRS surface.
- Provides `insertFormatTagIntoSource(...)`, which applies a static editor format tag such as `bold`, `italic`,
  `underline`, `strikethrough`, `highlight`, or `break` through `SetTag`, then returns both canonical RAW source and
  an editor HTML projection for the live LVRS surface. The session keeps the loaded `.wsnbody` RAW source as the
  format mutation basis and maps rendered break tags such as `<next />`/`<br>` as one logical newline. QML also passes
  `selectedText` so the session can repair a drifted RichText selection offset before mutating RAW source.

## Guardrails

- The editor file path must not point at the raw `.wsnbody` XML document.
- LVRS `TextEditor` is a rich-text surface, so loaded source must be projected to editor HTML with explicit line breaks
  before it reaches `filePath`.
- `.wsnbody` parsing and serialization stay in C++ note/editor model code, not QML.
- Format tag allow-list and mutation policy stay in `SetTag`; QML may only pass the requested tag name, current
  cursor/selection metadata, and selected visible text into this session boundary.
- Inline format mutation must not discard existing RAW wrapper tags just because the editor HTML projection no longer
  exposes them as visible text.
- Resource insertion must consume only imported package metadata. Clipboard MIME detection and `.wsresource` package
  persistence stay in `InAppClipboard`.

## 한국어

- 이 객체는 선택된 노트를 LVRS `TextEditor`에 연결하기 위한 C++ 세션 경계다.
- `editorFilePath`는 `.wsnbody` 원문이 아니라 editor HTML session file이어야 한다.
- `parsedLineCount`는 canonical RAW source line metadata이며 QML이 직접 파일을 읽거나 파싱하지 않게 한다.
  거터의 실제 row 개수는 이 값만 따르며, LVRS rendered wrap-line count를 따르지 않는다.
- LVRS가 session file 저장을 끝내면 `persistEditorFile(...)`이 다시 `.wsnbody` 저장 경로로 넘긴다.
- `insertImportedResourcesIntoSource(...)`는 `InAppClipboard`가 이미 `.wsresource`로 등록한 metadata만 받아
  canonical RAW `<resource ... />` 참조를 현재 커서/선택 위치에 삽입한다. clipboard MIME 판별과 package
  persistence는 이 세션의 책임이 아니다.
- `bold`, `italic`, `underline`, `strikethrough`, `highlight`, `break` 같은 포맷 태그는
  `insertFormatTagIntoSource(...)`가 `SetTag`를 통해 RAW source와 editor HTML projection을 함께 계산한다.
  로드된 `.wsnbody` RAW source가 mutation 기준이다. `<next />`/`<br>` 같은 source-level break는 selection 논리
  좌표에서 newline 1글자로 취급한다. LVRS RichText selection 좌표가 밀리면 함께 전달된 selected text로 실제 RAW
  visible 범위를 다시 찾는다.
