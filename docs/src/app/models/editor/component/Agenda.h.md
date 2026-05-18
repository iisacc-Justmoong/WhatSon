# `src/app/models/editor/component/Agenda.h`

## Responsibility

Declares the agenda editor component contract for static RAW tag insertion and Figma-backed editor rendering.

## Current Contract

- `AgendaStaticTag` describes one agenda-family paired RAW source template.
- `AgendaDescriptor` describes one rendered agenda frame: original source, displayed date/time, task children, and the
  editor viewport hint.
- `AgendaTaskItem` describes one rendered task item. It preserves the original task source, rendered inline HTML, and
  `done` state.
- `AgendaSourceRange` / `AgendaTaskSourceRange` expose paired-source ranges so persistence can render agenda/task
  wrappers without owning their parsing rules.
- `Agenda::staticTagNames()` returns the agenda-owned static tag names in insertion-menu order: `agenda`, then `task`.
- `Agenda::staticTagFor(...)` normalizes requested names and returns the paired source tokens for `agenda` and `task`.
- `Agenda::renderHtml(...)` projects `<agenda>` source to the editor's Figma frame (`279:7854`), with task items
  rendered as checkbox rows.
- `agenda` inserts `<agenda date="<current yyyy-MM-dd>" time="<current HH-mm>"><task done=false>` and
  `</task></agenda>` so an empty Agenda insertion places the cursor inside the first nested task body.
- `task` inserts `<task done=false>` and `</task>` for direct task wrapper edits.
- An agenda source can contain any integer number of child task wrappers between its opening and closing tags.

## 한국어

- 이 헤더는 Agenda 계열 정적 RAW tag 삽입 계약을 선언한다.
- `SetTag`는 전체 정적 태그 입력 흐름과 source mutation을 계속 맡지만, `agenda`와 `task`의 이름/토큰 정의는
  이 컴포넌트를 통해 얻는다.
- editor projection에서는 같은 컴포넌트가 Figma `279:7854` 프레임과 task checkbox row 렌더링 계약도 함께
  제공한다.
