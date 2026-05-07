# `src/app/models/editor/lineNumber`

## Scope

C++ model objects for the note editor's logical line-number rail.

## Child Files

- `ContentsLineNumberRailMetrics.hpp`
- `ContentsLineNumberRailMetrics.cpp`

## Current Contract

- `ContentsLineNumberRailMetrics` owns logical line-number row construction for the note editor.
- QML supplies view-owned inputs only: the current source snapshot, parser/renderer block metadata, measured row
  geometry snapshots, and LVRS line-height/width values.
- The C++ object de-duplicates iiHtmlBlock-derived block entries, derives row ranges from authoritative `sourceText`
  through its internal logical text bridge, maps each logical line back to RAW source offsets, marks resource rows from
  renderer metadata, combines those logical ranges with supplied row geometry snapshots, and publishes final
  `{ number, sourceStart, sourceEnd, y, height }` rows. The QML-facing `logicalText` property remains a compatibility
  input for change notification, but it is not the row-construction authority.
- The line-number object must not own or call TextEdit, cursor, selection, resource overlay, or QQuickItem objects
  directly. Surface measurement belongs to the geometry adapter and enters this object only as value snapshots.
- Published rows use measured geometry snapshots, but they must remain non-overlapping in the final gutter. A tall
  resource frame cannot turn into multiple gutter allocations, and rows clamped out of that frame advance the next
  minimum row top by their published height so later text/blank rows keep their own visible slots.
- A wrapped paragraph remains one logical number while its row height follows the visible wrapped height. Atomic
  resource blocks remain one logical number and one gutter-line allocation because parser-owned resource blocks are
  treated as non-text blocks, not as internal text lines.
- Rendered resource frames affect line-number placement only as a structured visual-block height delta supplied by the
  geometry adapter.
  That delta is anchored against the next plain logical row's measured base y, not the placeholder line-box height. If
  the next base y is not yet measurable, the full frame height is used as the row advance. The adapter also clamps any
  measured row top that still falls inside the active resource frame to the frame bottom before this object consumes it.
  Consecutive post-resource rows that share the same measured top are separated by one published row height instead of
  being painted on top of one another.
  Ordinary text row positions still come from the plain logical display geometry rather than RichText overlay rows or
  whole-document rendered `contentHeight`.
- The object does not mutate `.wsnbody` source and does not parse XML; it consumes parser/renderer metadata already
  produced by the editor projection pipeline.

## 한국어

- 대상: `src/app/models/editor/lineNumber`
- 역할: 노트 편집기 왼쪽 줄 번호 rail의 논리 줄 row 계산을 C++에서 담당한다.
- 기준: QML은 입력 데이터와 geometry snapshot을 넘기는 뷰 역할만 한다. 블록 de-dupe, `sourceText` 기반 논리 줄
  분할, source/logical offset 매핑, 최종 y/height row 생성은 `ContentsLineNumberRailMetrics`가 담당한다.
  `logicalText` property는 호환 입력일 뿐 정상 QML 경로의 source of truth가 아니다.
- 격리: `ContentsLineNumberRailMetrics`는 TextEdit, cursor, selection, resource overlay 객체를 직접 참조하지 않고
  측정된 row geometry 값만 사용한다.
- 위치: 각 row의 y/height는 geometry snapshot을 기준으로 결정하되, 최종 거터에서는 서로 겹치지 않아야 한다.
  resource frame 밖으로 clamp된 row 뒤의 다음 row는 이전 published row height만큼 아래로 배치한다.
- wrap: 긴 paragraph가 여러 시각 줄로 접혀도 번호는 하나이며, row height만 실제 표시 높이를 따른다.
- resource: rendered resource frame은 이후 row를 아래로 놓기 위한 structured visual-block height delta만 제공하며, 일반 텍스트 row의
  위치는 plain logical display geometry를 기준으로 유지한다. geometry adapter가 resource frame 내부 probe row를
  frame bottom으로 clamp한 뒤 전달하므로 내부 placeholder 줄이 거터 anchor가 되지 않는다. 같은 frame bottom으로
  clamp된 연속 row는 서로 한 줄 높이만큼 분리된다.
