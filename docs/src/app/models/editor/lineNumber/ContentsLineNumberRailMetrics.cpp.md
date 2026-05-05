# `src/app/models/editor/lineNumber/ContentsLineNumberRailMetrics.cpp`

## Responsibility

Implements logical line-number rail row construction outside QML.

## Behavior

- Normalizes renderer-owned block metadata into one row source per logical block token.
- Splits text groups by newline and `logicalLineCountHint`; duplicate iiHtmlBlock entries from the same token are
  ignored.
- Converts source offsets to logical display offsets through the supplied `logicalToSourceOffsets` table.
- Reads Qt text geometry through `positionToRectangle(...)` on the supplied text item. This keeps the actual view
  object responsible for measuring text while the row-building policy stays in C++.
- Resource blocks use the rendered overlay geometry so a rendered frame counts as one logical row with the frame's
  visible height.

## 한국어

이 구현은 QML에 있던 논리 줄 분할과 row y/height 생성을 C++로 이동한 것이다. QML은 TextEdit 객체를 넘겨
측정 표면만 제공하고, 정책 결정은 이 파일이 담당한다.
