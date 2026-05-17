# `src/app/models/editor/component/Callout.h`

## Responsibility

Declares the editor callout visual component contract.

## API

- `CalloutDescriptor::sourceText` stores the original paired `<callout>...</callout>` source fragment used for marker
  identity and attribute preservation.
- `CalloutDescriptor::contentHtml` stores the already-rendered inner editor HTML.
- `Callout::designWidth()` returns the Figma node `280:7897` baseline width (`295`).
- `Callout::sourceMarker(...)` hex-encodes source fragments for HTML comment markers.
- `Callout::renderHtml(...)` emits the structured editor HTML row consumed by `WhatSonNoteBodyPersistence`.

## 한국어

- 이 헤더는 callout source wrapper의 시각 렌더링 API를 정의한다.
- 실제 저장 정책은 `WhatSonNoteBodyPersistence`가 맡고, 이 컴포넌트는 editor HTML 표시 계약만 소유한다.
