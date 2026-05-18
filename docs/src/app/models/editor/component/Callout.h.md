# `src/app/models/editor/component/Callout.h`

## Responsibility

Declares the editor callout visual component and boundary-edit contract.

## API

- `CalloutDescriptor::sourceText` stores the original paired `<callout>...</callout>` source fragment used for marker
  identity and attribute preservation.
- `CalloutDescriptor::contentHtml` stores the already-rendered inner editor HTML.
- `CalloutSourceRange` describes one paired `<callout>...</callout>` source range, including opening tag, editable
  content, and closing tag spans.
- `CalloutBoundaryEdit` carries a planned source mutation and source cursor position for callout boundary keys.
- `Callout::designWidth()` returns the Figma node `280:7897` baseline width (`295`).
- `Callout::sourceMarker(...)` hex-encodes source fragments for HTML comment markers.
- `Callout::renderHtml(...)` emits the structured editor HTML row consumed by `WhatSonNoteBodyPersistence`.
- `Callout::sourceRanges(...)` parses paired callout ranges from canonical editor RAW source.
- `Callout::sourceVisibleCursorForDecoratedCursor(...)` and
  `Callout::decoratedContentStartForVisibleCursor(...)` convert between source-visible cursor positions and decorated
  Qt rich-text positions that include generated frame chrome.
- `Callout::backspaceAtVisibleContentStart(...)`, `Callout::enterBeforeContentChrome(...)`, and
  `Callout::enterInsideVisibleCursor(...)` plan the source edits for Backspace/Enter at callout boundaries. The editor
  session applies those edits, reprojects HTML, and updates active-note state.

## 한국어

- 이 헤더는 callout source wrapper의 시각 렌더링 API와 boundary key 편집 계획 API를 정의한다.
- 실제 저장 정책은 `WhatSonNoteBodyPersistence`가 맡고, active note 상태 갱신과 editor HTML 재투영은
  `NoteEditorDocumentSession`이 맡는다. 이 컴포넌트는 콜아웃 source 범위, 장식 커서 매핑, Backspace/Enter
  boundary rule을 순수 계약으로 소유한다.
