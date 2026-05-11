# `src/app/qml/view/contents/Gutter.qml`

## Responsibility

`Gutter.qml` is the only contents-side gutter view. It is a QML-only visual rail for line numbers.

## Contract

- Root type: `Item`.
- Imports: `QtQuick` and `LVRS 1.0 as LV`.
- It exposes only view inputs: `sourceFilePath`, `selectedNoteId`, `selectedNoteDirectoryPath`, `parsedLineCount`,
  `lineCount`, `fallbackLineHeight`, `lineMetricProvider`, `lineMetricsRevision`, `contentY`, colors, and
  `showLineNumbers`.
- The caller is responsible for passing the selected editor session file and parsed line count. The gutter only renders
  line numbers and follows the editor viewport offset.
- Its width reserves space for at least six line-number digits and grows beyond that when `lineCount` needs more digits.
- Each line-number delegate calls `lineMetricProvider(index)` and uses the returned editor line `y` and `height`.
  `fallbackLineHeight` is used only when the editor surface has not exposed a measurable peer line yet.
- It does not draw a separator between the gutter and editor.
- It does not own parser, projection, persistence, editor session, or note mutation behavior.

## 한국어

- 기준: contents 내부 QML에서 허용되는 세 뷰 중 거터 담당 파일이다.
- 동작: 선택 노트의 session file 경로와 C++ parsed line count를 입력으로 받아 line number rail만 표시한다.
- 동작: 거터 폭은 최소 6자리 line number를 표시할 수 있는 폭을 예약하고, 7자리 이상이 필요하면 더 넓어진다.
- 동작: 각 줄 번호는 자신의 index를 sibling `TextEditor.qml` metric provider에 넘겨 대응 editor line의 `y`와
  `height`를 받아 배치한다.
- 동작: 거터 우측과 에디터 좌측 사이의 분리선은 그리지 않는다.
- 금지: source snapshot, projection, rendering pipeline, persistence, editor backend wiring을 추가하지 않는다.
