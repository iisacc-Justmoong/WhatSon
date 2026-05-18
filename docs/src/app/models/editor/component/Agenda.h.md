# `src/app/models/editor/component/Agenda.h`

## Responsibility

Declares the agenda editor component contract for static RAW tag insertion.

## Current Contract

- `AgendaStaticTag` describes one agenda-family paired RAW source template.
- `Agenda::staticTagNames()` returns the agenda-owned static tag names in insertion-menu order: `agenda`, then `task`.
- `Agenda::staticTagFor(...)` normalizes requested names and returns the paired source tokens for `agenda` and `task`.
- `agenda` inserts `<agenda><task>` and `</task></agenda>` so an empty Agenda insertion places the cursor inside the
  nested task body.
- `task` inserts `<task>` and `</task>` for direct task wrapper edits.

## 한국어

- 이 헤더는 Agenda 계열 정적 RAW tag 삽입 계약을 선언한다.
- `SetTag`는 전체 정적 태그 입력 흐름과 source mutation을 계속 맡지만, `agenda`와 `task`의 이름/토큰 정의는
  이 컴포넌트를 통해 얻는다.
