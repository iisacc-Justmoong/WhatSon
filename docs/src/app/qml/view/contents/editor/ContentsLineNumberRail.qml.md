# `src/app/qml/view/contents/editor/ContentsLineNumberRail.qml`

## Responsibility

Renders the logical-line-number gutter for the live note editor surface.

## Current Contract

- `rows` is a view-only array supplied by `ContentsStructuredDocumentFlow.editorLogicalGutterRows`.
- Each row represents one full-document logical text line, not one wrapped visual text row.
- A wrapped logical line still paints one number; that row's `height` is the measured editor geometry height occupied by
  the wrapped text.
- Atomic/tall blocks, such as rendered resource frames, still count as one logical line while their row height follows
  the block's rendered vertical footprint.
- The component has no background, border, parser logic, input handler, or persistence authority. It must stay visually
  merged with the editor body and scroll only because its parent viewport scrolls.

## Pipeline Position

`ContentsInlineFormatEditor.qml` binds editor geometry and projection metadata into the C++
`ContentsLineNumberRailMetrics` object, which publishes row objects with `number`, `y`, and `height`.
`ContentsDisplayView.qml` mounts this line-number rail inside the same `Flickable` content item as
`ContentsStructuredDocumentFlow.qml`, so the gutter and body share one y-coordinate system.

## 한국어

- 대상: `ContentsLineNumberRail.qml`
- 역할: 노트 본문 왼쪽에 논리 줄 번호를 표시한다.
- 기준: wrap은 번호를 늘리지 않는다. 긴 `<paragraph>`가 세 시각 줄로 접혀도 거터에는 `1` 하나만 있고, 그
  행의 높이가 세 줄 높이만큼 커진다. RAW 태그가 숨겨진 뒤 남는 빈 논리 줄도 번호 슬롯으로 유지한다.
  이 row 계산은 C++ `ContentsLineNumberRailMetrics`가 담당한다.
- 리소스 프레임 같은 tall block도 논리 줄 하나로 카운트하되, 표시 높이만큼 거터 행 높이를 가진다.
- 배경색과 입력 처리는 없으며, 본문과 같은 스크롤 콘텐츠 안에서만 움직인다.
