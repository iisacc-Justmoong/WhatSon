# `src/app/models/editor/component/Agenda.cpp`

## Responsibility

Implements the agenda-family static RAW tag template lookup and editor HTML renderer.

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
- The visual values retained from Figma are `#262728` surface, `#343536` stroke, `12px` radius, `8px` padding, `8px`
  header/task gap, and `4px` task gap.
- Rendered task items use checkbox chrome matching the LVRS `CheckBox` defaults (`17px` box, `3.5px` radius, `6px`
  text gap) while `.wsnbody` still stores only `<task done=...>...</task>`.
- The renderer writes source markers so direct rich-text saves and Qt/LVRS reserialization can recover the canonical
  `<agenda>/<task>` source instead of degrading the frame to callout-colored paragraphs.
- `SetTag` consumes these descriptors and applies the generic selection wrapping, toggle-off, body-document
  serialization, and invalid-tag result behavior.

## 한국어

- 이 구현은 `agenda`와 `task` 정적 삽입 토큰의 단일 소스다. agenda 삽입 토큰은 삽입 시점의 로컬 `date`,
  `time` 값과 첫 `<task done=false>` child를 포함한다.
- Figma `279:7854` editor frame 렌더링도 여기서 담당한다. source text 변형, selection 처리,
  `.wsnbody` 재직렬화는 각각 `SetTag`와 note-body persistence 경계가 담당한다.
