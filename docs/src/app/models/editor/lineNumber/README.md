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
- Published rows must be vertically monotonic. If a measured row snapshot collapses a later row to the first row y, the
  model falls back to the previous row bottom so line numbers remain aligned as separate rows.
- A wrapped paragraph remains one logical number while its row height follows the visible wrapped height. Atomic
  resource blocks remain one logical number while their height is measured from the rendered resource overlay.
- The object does not mutate `.wsnbody` source and does not parse XML; it consumes parser/renderer metadata already
  produced by the editor projection pipeline.

## 한국어

- 대상: `src/app/models/editor/lineNumber`
- 역할: 노트 편집기 왼쪽 줄 번호 rail의 논리 줄 row 계산을 C++에서 담당한다.
- 기준: QML은 입력 데이터와 geometry snapshot을 넘기는 뷰 역할만 한다. 블록 de-dupe, 전체 `logicalText` 기반
  논리 줄 분할, source/logical offset 매핑, 최종 y/height row 생성은 `ContentsLineNumberRailMetrics`가 담당한다.
- 격리: `ContentsLineNumberRailMetrics`는 TextEdit, cursor, selection, resource overlay 객체를 직접 참조하지 않고
  측정된 row geometry 값만 사용한다.
- 위치: 각 row의 y는 실제 geometry를 우선하되, geometry가 같은 y로 붕괴하면 이전 row 아래로 보정한다.
- wrap: 긴 paragraph가 여러 시각 줄로 접혀도 번호는 하나이며, row height만 실제 표시 높이를 따른다.
