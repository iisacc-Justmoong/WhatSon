# `src/app/models/editor/component/Agenda.h`

## Responsibility

Declares the agenda editor component contract for static RAW tag insertion, Figma-backed editor rendering, and
agenda-local boundary key source edits.

## Current Contract

- `AgendaStaticTag` describes one agenda-family paired RAW source template.
- `AgendaDescriptor` describes one rendered agenda frame: original source, displayed date/time, task children, and the
  editor viewport hint.
- `AgendaTaskItem` describes one rendered task item. It preserves the original task source, rendered inline HTML, and
  `done` state.
- `AgendaSourceRange` / `AgendaTaskSourceRange` expose paired-source ranges so persistence can render agenda/task
  wrappers without owning their parsing rules.
- `AgendaBoundaryEdit` carries a planned canonical source mutation for agenda boundary keys, including the next source
  cursor, optional target task index, and the "cursor below agenda" hint needed after exiting an empty trailing task.
- `Agenda::staticTagNames()` returns the agenda-owned static tag names in insertion-menu order: `agenda`, then `task`.
- `Agenda::staticTagFor(...)` normalizes requested names and returns the paired source tokens for `agenda` and `task`.
- `Agenda::renderHtml(...)` projects `<agenda>` source to the editor's Figma frame (`279:7854`), with task items
  rendered as rows containing a transparent checkbox slot for the QML `LV.CheckBox` overlay. The rich-text projection
  is table-backed so Qt paints one continuous fill-width frame surface rather than separate nested block backgrounds.
- `agenda` inserts `<agenda date="<current yyyy-MM-dd>" time="<current HH-mm>"><task done=false>` and
  `</task></agenda>` so an empty Agenda insertion places the cursor inside the first nested task body.
- `task` inserts `<task done=false>` and `</task>` for direct task wrapper edits.
- An agenda source can contain any integer number of child task wrappers between its opening and closing tags.
- `Agenda::backspaceAtFirstTaskContentStart(...)` plans removal of the whole agenda source block when the editor
  cursor is at the first task body start.
- `Agenda::enterInLastTask(...)` plans the last-task Enter behavior: non-empty trailing tasks append a new empty
  `<task done=false></task>`, while an empty trailing task is removed and the cursor exits to the line below the agenda.

## 한국어

- 이 헤더는 Agenda 계열 정적 RAW tag 삽입, editor frame 렌더링, agenda 내부 경계 키 source edit 계약을 선언한다.
- `SetTag`는 전체 정적 태그 입력 흐름과 source mutation을 계속 맡지만, `agenda`와 `task`의 이름/토큰 정의는
  이 컴포넌트를 통해 얻는다.
- editor projection에서는 같은 컴포넌트가 Figma `279:7854` 프레임과 task checkbox row 렌더링 계약도 함께
  제공한다. 실제 rich-text 출력은 Qt 렌더러에 맞춘 table frame으로, 배경이 줄 단위로 쪼개지지 않아야 한다.
- 첫 task body 시작점의 Backspace는 agenda 전체 제거로, 마지막 task의 Enter는 다음 task 생성 또는 빈 trailing
  task 제거 후 agenda 아래로의 cursor 이동으로 계획된다.
