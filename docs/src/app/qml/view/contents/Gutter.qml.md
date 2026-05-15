# `src/app/qml/view/contents/Gutter.qml`

## Responsibility

`Gutter.qml` is the only contents-side gutter view. It is a QML-only visual rail for line numbers.

## Contract

- Root type: `Item`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- It exposes only view inputs: `sourceFilePath`, `selectedNoteId`, `selectedNoteDirectoryPath`, `parsedLineCount`,
  `lineCount`, `fallbackLineHeight`, `lineMetricProvider`, `lineMetricsRevision`, `currentLineIndex`, `contentY`,
  colors, and `showLineNumbers`.
- The caller is responsible for passing the selected editor session file and the parsed source line count. `lineCount`
  controls delegate creation and must come from canonical note/source lines, not wrapped visual rows.
- Its width reserves space for at least six line-number digits and grows beyond that when `lineCount` needs more digits.
- Each line-number delegate calls `lineMetricProvider(index)` to place the number at the rendered start of that logical
  source line. Wrapped continuation rows do not create delegates and therefore remain unnumbered. `fallbackLineHeight`
  is used only when the editor surface has not exposed a measurable peer line yet.
- If a logical source line has a tall rendered block such as an image resource frame, the delegate may span the whole
  block height, but the visible line number and current-line indicator stay inside the top fallback-line-height band.
  They must not be vertically centered inside the entire resource frame.
- The delegate whose index matches `currentLineIndex` draws a blue rounded pill bar on the left side of the gutter as
  the current cursor-line indicator. `currentLineIndex` is expected to be the sibling editor's logical cursor line.
- It does not draw a separator between the gutter and editor.
- It does not own parser, projection, persistence, editor session, or note mutation behavior.

## 한국어

- 기준: contents 내부 QML에서 허용되는 네 뷰 중 거터 담당 파일이다.
- 동작: 선택 노트의 session file 경로와 C++ parsed line count를 입력으로 받아 line number rail만 표시한다.
  실제 생성 개수는 `lineCount`가 담당하고, 이는 canonical source line 수다.
  시각적 wrap 행은 별도 line number를 만들지 않는다.
- 동작: 거터 폭은 최소 6자리 line number를 표시할 수 있는 폭을 예약하고, 7자리 이상이 필요하면 더 넓어진다.
- 동작: 각 줄 번호는 sibling `TextEditor.qml`의 `lineMetricProvider(index)`가 반환한 logical source line 시작
  위치에 배치된다. paragraph가 wrap되어 여러 시각 행이 되어도 continuation row에는 delegate가 없으므로 번호가
  찍히지 않는다.
- 동작: 이미지 resource frame처럼 한 source line이 큰 높이로 렌더되어도 line number와 현재 줄 indicator는
  전체 block 중앙이 아니라 상단 fallback line-height 밴드 안에 표시된다.
- 동작: `currentLineIndex`와 일치하는 줄 번호의 좌측에는 파란색 알약 모양 막대를 그려 현재 cursor line을
  표시한다. 이 값은 sibling editor의 logical cursor line 기준이어야 하며, paragraph wrap visual row를 별도
  줄로 세지 않는다.
- 동작: 거터 우측과 에디터 좌측 사이의 분리선은 그리지 않는다.
- 금지: source snapshot, projection, rendering pipeline, persistence, editor backend wiring을 추가하지 않는다.
