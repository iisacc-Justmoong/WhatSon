# `src/app/qml/view/contents/editor/ContentsLineNumberRail.qml`

## Responsibility

Renders the logical-line-number gutter for the live note editor surface.

## Current Contract

- `rows` is a view-only array supplied by `ContentsStructuredDocumentFlow.editorLogicalGutterRows`.
- `activeSourceCursorPosition`, `activeSelectionStart`, and `activeSelectionEnd` are RAW source offsets supplied by the
  editor host.
- Each row represents one full-document logical text line, not one wrapped visual text row.
- A wrapped logical line still paints one number; that row's `height` is the measured editor geometry height occupied by
  the wrapped text.
- Atomic/tall blocks, such as rendered resource frames, still count as one logical line and paint one gutter-line row;
  later rows are placed below the block by the geometry/metrics pipeline, using the frame bottom rather than the hidden
  placeholder line-box height.
- The rail publishes `preferredWidth`, which keeps the number column and right padding while reducing the formerly
  implicit blank area to the left of the number column by half.
- The rail paints a blue `LV.Theme.accentBlue` indicator bar for the row containing the collapsed cursor. For non-empty
  selections it paints the same bar for rows whose `sourceStart/sourceEnd` range intersects the selected RAW range.
- The component has no background, border, parser logic, input handler, or persistence authority. It must stay visually
  merged with the editor body and scroll only because its parent viewport scrolls.

## Pipeline Position

`ContentsInlineFormatEditor.qml` binds editor geometry and projection metadata into the C++
`ContentsLineNumberRailMetrics` object, which publishes row objects with `number`, `y`, and `height`.
`ContentViewLayout.qml` mounts this line-number rail inside the same `Flickable` content item as
`ContentsStructuredDocumentFlow.qml`, so the gutter and body share one y-coordinate system.

## 한국어

- 대상: `ContentsLineNumberRail.qml`
- 역할: 노트 본문 왼쪽에 논리 줄 번호를 표시한다.
- 기준: wrap은 번호를 늘리지 않는다. 긴 `<paragraph>`가 세 시각 줄로 접혀도 거터에는 `1` 하나만 있고, 그
  행의 높이가 세 줄 높이만큼 커진다. RAW 태그가 숨겨진 뒤 남는 빈 논리 줄도 번호 슬롯으로 유지한다.
  이 row 계산은 C++ `ContentsLineNumberRailMetrics`가 담당한다.
- 폭: 번호 컬럼과 오른쪽 8px inset은 유지하되, 번호 왼쪽의 빈 영역은 기존 `LV.Theme.buttonMinWidth`
  기준 implicit blank의 절반만 사용한다.
- active line: host가 전달한 RAW cursor/selection offset을 각 row의 `sourceStart/sourceEnd`와 비교해, 커서가
  있거나 selection과 교차하는 줄에는 파란색 accent bar를 표시한다.
- 리소스 프레임 같은 tall block도 논리 줄 하나와 거터 한 줄로만 카운트하며, 이후 row는 geometry/metrics
  pipeline이 프레임 아래 위치로 배치한다.
- 배경색과 입력 처리는 없으며, 본문과 같은 스크롤 콘텐츠 안에서만 움직인다.
