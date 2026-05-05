# `src/app/models/editor/lineNumber`

## Scope

C++ model objects for the note editor's logical line-number rail.

## Child Files

- `ContentsLineNumberRailMetrics.hpp`
- `ContentsLineNumberRailMetrics.cpp`

## Current Contract

- `ContentsLineNumberRailMetrics` owns logical line-number row construction for the note editor.
- QML supplies view-owned inputs only: the current source snapshot, renderer-owned `normalizedHtmlBlocks`, the
  logical-to-source offset table, the TextEdit geometry objects, and LVRS line-height/width values.
- The C++ object de-duplicates iiHtmlBlock-derived block entries, splits logical lines from `sourceText` and
  `logicalLineCountHint`, maps source offsets to visible logical offsets, asks the supplied Qt text item for
  `positionToRectangle(...)`, and publishes final `{ number, sourceStart, sourceEnd, y, height }` rows.
- A wrapped paragraph remains one logical number while its row height follows the visible wrapped height. Atomic
  resource blocks remain one logical number while their height is measured from the rendered resource overlay.
- The object does not mutate `.wsnbody` source and does not parse XML; it consumes parser/renderer metadata already
  produced by the editor projection pipeline.

## 한국어

- 대상: `src/app/models/editor/lineNumber`
- 역할: 노트 편집기 왼쪽 줄 번호 rail의 논리 줄 row 계산을 C++에서 담당한다.
- 기준: QML은 TextEdit/overlay 객체와 입력 데이터를 넘기는 뷰 역할만 한다. 블록 de-dupe, 논리 줄 분할,
  source/logical offset 매핑, 최종 y/height row 생성은 `ContentsLineNumberRailMetrics`가 담당한다.
- wrap: 긴 paragraph가 여러 시각 줄로 접혀도 번호는 하나이며, row height만 실제 표시 높이를 따른다.
