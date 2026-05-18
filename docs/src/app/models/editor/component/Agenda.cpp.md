# `src/app/models/editor/component/Agenda.cpp`

## Responsibility

Implements the agenda-family static RAW tag template lookup, editor HTML renderer, and agenda-local boundary key edit
planning.

## Runtime Behavior

- Tag-name lookup is case-insensitive and ignores non-alphanumeric characters, matching the normalization behavior used
  by `SetTag`.
- The component returns valid descriptors only for `agenda` and `task`; other static body tags remain owned by their
  existing editor components or by `SetTag`.
- The agenda template fills `date` and `time` from the local insertion time using `yyyy-MM-dd` and `HH-mm`, and creates
  one default `<task done=false>` child. Additional task children are still ordinary RAW source inside the same agenda
  wrapper.
- `renderHtml(...)` renders agenda source as the Figma `279:7854` visual frame, but it does not persist the Figma node
  width/height as static frame dimensions. The emitted root is `width="100%"`, `height:auto`,
  `data-frame-width-mode="fill"`, and `data-frame-height-mode="hug-contents"` so the agenda fills the editor width
  and grows only to contain its task children.
- The editor renderer uses a table-backed rich-text frame instead of nested block `div` rows because Qt rich text
  paints nested block backgrounds as separate text-line strips. The table root keeps the full-width surface continuous,
  and the header uses two cells so the date remains right-aligned like the Figma auto-layout `space-between` row.
- The visual values retained from Figma are `#262728` surface, `#343536` stroke, `12px` radius, `8px` padding, `8px`
  header/task gap, and `4px` task gap.
- Rendered task items reserve a transparent checkbox slot matching the LVRS `CheckBox` default size (`17px` box,
  `6px` text gap). The visual checkbox itself is not a rich-text PNG; `ContentViewLayout.qml` places the real
  `LV.CheckBox` over that slot while `.wsnbody` still stores only `<task done=...>...</task>`.
- The renderer writes source markers so direct rich-text saves and Qt/LVRS reserialization can recover the canonical
  `<agenda>/<task>` source instead of degrading the frame to callout-colored paragraphs.
- Boundary key edit planning stays in this component because it depends on agenda/task paired-source ranges. Backspace
  at the first task content start removes the full agenda source block with adjacent line-break cleanup. Enter in the
  last task appends a fresh empty task when the current last task has content; Enter in an empty last task deletes that
  empty task and requests the cursor below the agenda.
- `SetTag` consumes these descriptors and applies the generic selection wrapping, toggle-off, body-document
  serialization, and invalid-tag result behavior.

## 한국어

- 이 구현은 `agenda`와 `task` 정적 삽입 토큰의 단일 소스다. agenda 삽입 토큰은 삽입 시점의 로컬 `date`,
  `time` 값과 첫 `<task done=false>` child를 포함한다.
- Figma `279:7854` editor frame 렌더링도 여기서 담당한다. 일반 삽입/selection 처리는 `SetTag`가 맡지만, agenda
  task 경계 키의 source edit 계획은 agenda/task range를 아는 이 컴포넌트가 맡는다. `.wsnbody` 재직렬화는
  note-body persistence 경계가 담당한다.
- Qt rich text에서 nested block `div`가 줄별 배경처럼 쪼개져 보이지 않도록, editor projection은 table 기반
  프레임을 사용한다. header는 좌측 `Agenda`와 우측 날짜 셀로 분리해 Figma의 `space-between` 배치를 보존한다.
- 첫 task 시작점 Backspace는 agenda source block 전체를 제거하고, 마지막 task Enter는 새 task 생성 또는 빈
  trailing task 제거 후 agenda 아래 cursor 이동으로 계획된다.
