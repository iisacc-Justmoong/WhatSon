# `src/app/models/editor/component/Break.cpp`

## Responsibility

Implements the standalone editor break source-line component.

## Runtime Behavior

- The canonical editor source token is `</break>`.
- `<break/>` and legacy `<hr/>` inputs are accepted only as standalone source lines and normalize back to `</break>`.
- Rendering a break line returns an empty fragment. When the note body projection joins logical source lines with
  `<br/>`, that empty fragment becomes a visible blank editor line at the same logical line number.
- The component does not own persistence by itself; `WhatSonNoteBodyPersistence` stores the token as canonical
  `.wsnbody` XML `<break/>` and reprojects it through this component.

## 한국어

- 이 구현은 `</break>` source line을 노트 에디터의 논리 빈 줄로 변환한다.
- 태그 문자열은 editor HTML에 노출되지 않는다.
- `.wsnbody` 저장은 계속 `<break/>`를 사용하고, editor source projection은 `</break>`를 기준으로 한다.
