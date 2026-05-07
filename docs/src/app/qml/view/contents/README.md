# `src/app/qml/view/contents`

## Role

This directory is the single contents-view namespace for editor-facing QML.

It merges the standalone Figma contents-surface artifacts and the runtime editor surface under
`src/app/qml/view/contents` so `src/app/qml/contents` and `src/app/qml/view/content` cannot diverge.

## Current Files

- `ContentsView.qml`: Figma node `155:4561` root frame and LVRS layout host.
- `EditorView.qml`: read-only text projection for Figma node `155:5352`.
- `Minimap.qml`: minimap row rail for Figma node `352:8626`.
- `editor/`: runtime note editor view components, including `ContentsLineNumberRail.qml`,
  `ContentsStructuredDocumentFlow.qml`, and the inline/resource editor surfaces.

## Boundary

- QML in this directory is presentation-only.
- It does not own note persistence, parser state, or editor mutation authority.
- Every QML file imports LVRS and expresses colors, spacing, typography, and fixed rails through `LV.Theme` tokens.
- Figma dimensions are preserved through token compositions instead of literal pixel values.
- Standalone `ContentsView.qml` and the runtime note editor chrome in `ContentViewLayout.qml` both apply
  `LV.Theme.gap8` as a shared top and bottom chrome inset, so editor, gutter, and minimap surfaces start and end inside
  the same vertical padding.
- Minimap visible-line measurement is delegated to `ContentsEditorVisualLineMetrics`, and minimap arithmetic is
  delegated to `ContentsMinimapLayoutMetrics`; the QML files render resolved values. Runtime editor hosts must feed
  those models with visible TextEdit geometry and primitive LVRS token values, then feed `Minimap.qml` with matching
  per-row width ratios and scroll-drag enablement. `Minimap.qml` renders rows inside an 8px left/right inset, does not
  render a viewport thumb or scrollbar chrome, and drag movement is emitted as a direct pixel delta instead of a
  normalized scrollbar ratio.
- Runtime editor gutters are view-only QML chrome. They receive logical-line rows from
  `ContentsLineNumberRailMetrics`, a C++ editor model that consumes authoritative source text, structured projection
  metadata, and measured geometry snapshots. The rail must remain inside the body viewport rather than becoming a detached
  overlay, its own `preferredWidth` halves the leading blank area before the line-number column, and cursor/selection
  offsets only drive the blue active-line bar.
- `ContentViewLayout.qml` imports this directory as `ContentsChrome` and reuses `Minimap.qml` from this namespace.

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
- padding: standalone `ContentsView.qml`와 runtime `ContentViewLayout.qml`의 note editor chrome은 모두 상하 `LV.Theme.gap8` inset을
  적용해 에디터, 거터, 미니맵이 같은 vertical padding 안에서 렌더되도록 한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다. 런타임 미니맵 행 수와 행별 폭은 실제 에디터에 표시된 wrap 결과 줄과 표시 길이를 따르며, 행 폭은 좌우 8px inset을 제외한 내부 rail 기준으로 계산한다. 미니맵 세로 드래그는 host viewport에 같은 pixel delta로 연결하되 별도 viewport thumb이나 스크롤바 chrome은 표시하지 않는다.
- gutter: 런타임 거터의 논리 줄 번호와 각 번호 행의 y/height는 C++ `ContentsLineNumberRailMetrics`가
  생성한다. QML rail은 본문 viewport 안에 함께 배치되어 스크롤 좌표계를 공유해야 하며, 라인 넘버 왼쪽
  빈 영역은 rail의 `preferredWidth`에서 기존 implicit blank의 절반으로 제한한다. 커서 또는 selection과
  교차하는 줄에는 파란색 active-line bar를 표시한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
