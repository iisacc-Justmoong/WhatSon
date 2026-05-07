# `src/app/models/editor/lineNumber`

## Scope

C++ model objects for the note editor's logical line-number rail.

## Child Files

- `ContentsLineNumberRailMetrics.hpp`
- `ContentsLineNumberRailMetrics.cpp`

## Current Contract

- `ContentsLineNumberRailMetrics` owns logical line-number row construction for the note editor.
- QML supplies view-owned inputs only: the current source snapshot, renderer-owned `normalizedHtmlBlocks`, measured row
  geometry snapshots, and LVRS line-height/width values.
- The C++ object de-duplicates iiHtmlBlock-derived block entries, splits row ranges from the full `logicalText`
  projection, maps each logical line back to RAW source offsets through its internal logical text bridge, marks resource
  rows from renderer metadata, combines those logical ranges with supplied row geometry snapshots, and publishes final
  `{ number, sourceStart, sourceEnd, y, height }` rows.
- The line-number object must not own or call TextEdit, cursor, selection, resource overlay, or QQuickItem objects
  directly. Surface measurement belongs to the geometry adapter and enters this object only as value snapshots.
- Published rows must be independent. Each row keeps its measured `y`; a tall resource frame must not push later
  gutter rows down through previous-row fallback state.
- A wrapped paragraph remains one logical number while its row height follows the visible wrapped height. Atomic
  resource blocks remain one logical number and one gutter-line allocation because the logical text bridge exposes
  them as one U+FFFC placeholder, not as internal text lines.
- Rendered resource frames affect line-number placement only as an image-height delta supplied by the geometry adapter.
  That delta is anchored against the next plain logical row's measured base y, not the placeholder line-box height. If
  the next base y is not yet measurable, the full frame height is used as the row advance. The adapter also clamps any
  measured row top that still falls inside the active resource frame to the frame bottom before this object consumes it.
  Ordinary text row positions still come from the plain logical display geometry rather than RichText overlay rows or
  whole-document rendered `contentHeight`.
- The object does not mutate `.wsnbody` source and does not parse XML; it consumes parser/renderer metadata already
  produced by the editor projection pipeline.

## 한국어

- 대상: `src/app/models/editor/lineNumber`
- 역할: 노트 편집기 왼쪽 줄 번호 rail의 논리 줄 row 계산을 C++에서 담당한다.
- 기준: QML은 입력 데이터와 geometry snapshot을 넘기는 뷰 역할만 한다. 블록 de-dupe, 전체 `logicalText` 기반
  논리 줄 분할, source/logical offset 매핑, 최종 y/height row 생성은 `ContentsLineNumberRailMetrics`가 담당한다.
- 격리: `ContentsLineNumberRailMetrics`는 TextEdit, cursor, selection, resource overlay 객체를 직접 참조하지 않고
  측정된 row geometry 값만 사용한다.
- 위치: 각 row의 y/height는 해당 row의 geometry snapshot에서 독립적으로 결정하며, 이전 row의 bottom을 다음 row에
  전파하지 않는다.
- wrap: 긴 paragraph가 여러 시각 줄로 접혀도 번호는 하나이며, row height만 실제 표시 높이를 따른다.
- resource: rendered resource frame은 이후 row를 아래로 놓기 위한 image-height delta만 제공하며, 일반 텍스트 row의
  위치는 plain logical display geometry를 기준으로 유지한다. geometry adapter가 resource frame 내부 probe row를
  frame bottom으로 clamp한 뒤 전달하므로 내부 placeholder 줄이 거터 anchor가 되지 않는다.
