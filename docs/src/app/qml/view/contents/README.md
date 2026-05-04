# `src/app/qml/view/contents`

## Role

This directory is the single contents-view namespace for editor-facing QML.

It merges the standalone Figma contents-surface artifacts and the runtime editor surface under
`src/app/qml/view/contents` so `src/app/qml/contents` and `src/app/qml/view/content` cannot diverge.

## Current Files

- `ContentsView.qml`: Figma node `155:4561` root frame and LVRS layout host.
- `EditorView.qml`: read-only text projection for Figma node `155:5352`.
- `Minimap.qml`: minimap row rail for Figma node `352:8626`.
- `editor/`: runtime note editor host components, including `ContentsDisplayView.qml`,
  `ContentsStructuredDocumentFlow.qml`, and the inline/resource editor surfaces.

## Boundary

- QML in this directory is presentation-only.
- It does not own note persistence, parser state, or editor mutation authority.
- Every QML file imports LVRS and expresses colors, spacing, typography, and fixed rails through `LV.Theme` tokens.
- Figma dimensions are preserved through token compositions instead of literal pixel values.
- Minimap arithmetic is delegated to `ContentsMinimapLayoutMetrics`; the QML files render resolved values.
- `ContentsDisplayView.qml` imports the parent `view/contents` directory as `ContentsChrome` and reuses
  `Minimap.qml` from this namespace.

## Tests

- `test/cpp/suites/qml_contents_view_tests.cpp` locks the standalone composition contract and the read-only editor
  projection boundary.
- `test/cpp/suites/qml_editor_surface_policy_tests.cpp` locks the runtime editor host composition against this
  consolidated `view/contents` namespace.
- `test/cpp/suites/include_path_policy_tests.cpp` forbids reintroducing `src/app/qml/contents` or
  `src/app/qml/view/content`.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/contents`` (`docs/src/app/qml/view/contents/README.md`)
- 위치: `docs/src/app/qml/view/contents`
- 역할: 이 디렉터리는 standalone Figma contents surface와 runtime editor surface를 모두 담는 단일 QML contents view 네임스페이스다.
- 배치: `src/app/qml/contents`와 `src/app/qml/view/content`는 재도입하지 않고, `EditorView.qml`, `Minimap.qml`, `ContentsView.qml`, 그리고 `editor/*`를 모두 `src/app/qml/view/contents` 아래에 둔다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
