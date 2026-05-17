# `src/app/qml/view/contents`

## Role

This directory is the single contents-view namespace for content-surface QML.

Only the explicit contents files listed below are allowed here. Runtime composition still mounts only the implemented
gutter, image editor, text editor, and minimap surfaces; the other editor files are build-safe placeholders.

## Current Files

- `Gutter.qml`: logical line-number rail.
- `ImageEditor.qml`: image-resource viewer for the current `ResourcesListModel.currentResourceEntry`.
- `TextEditor.qml`: runtime editor surface; its root item is `LV.TextEditor`.
- `Minimap.qml`: visual minimap rail.
- `3DEditor.qml`, `AudioEditor.qml`, `BinEditor.qml`, `DocumentEditor.qml`, `VideoEditor.qml`: plain `Item`
  placeholders kept only so the QML module can compile while future resource viewers remain unimplemented.

## Boundary

- QML in this directory is presentation-only.
- It does not own note persistence, parser state, projection, resource metadata persistence, or editor mutation
  authority.
- Every QML file imports LVRS and expresses colors, spacing, typography, and fixed rails through `LV.Theme` tokens.
- Wrapper QML, standalone Figma frames, generic resource editor views, structured-document flow, projection, and
  persistence wiring are not allowed in this contents namespace.
- `ContentViewLayout.qml` owns composition outside this directory and currently mounts only `Gutter.qml`,
  `ImageEditor.qml`, `TextEditor.qml`, and `Minimap.qml` from this namespace.
- Placeholder editor files must stay as plain `Item` roots until a real composition path, tests, and resource-viewer
  contract are added.

## Tests

- `test/cpp/suites/qml_contents_view_tests.cpp` locks the allowed contents QML view contract and the image-resource
  viewer routing surface.
- `test/cpp/suites/qml_inline_format_editor_tests.cpp` locks the runtime editor surface to `TextEditor.qml` and
  `LV.TextEditor`, while allowing focused format shortcuts and the selected-text format context menu only in the outer
  `ContentViewLayout.qml` dispatch layer. It also locks the wrapper-level cursor restoration used after C++-computed
  resource or format document replacement.
- `test/cpp/suites/include_path_policy_tests.cpp` forbids reintroducing unlisted contents QML files or legacy contents
  directories.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/contents`` (`docs/src/app/qml/view/contents/README.md`)
- 위치: `docs/src/app/qml/view/contents`
- 역할: 이 디렉터리는 content-surface contents view 네임스페이스다.
- 배치: 실제 composition은 `Gutter.qml`, `ImageEditor.qml`, `TextEditor.qml`, `Minimap.qml` 네 파일만 사용한다.
  `3DEditor.qml`, `AudioEditor.qml`, `BinEditor.qml`, `DocumentEditor.qml`, `VideoEditor.qml`은 빌드 우회용 단순
  `Item` placeholder다.
- 주의: placeholder editor QML은 실제 viewer wiring을 갖지 않는다. 새 surface는 composition 경로, 테스트, resource
  viewer 계약을 먼저 갱신한 뒤 구현한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다. 런타임 본문 편집기 QML은 `LV.TextEditor`를 직접 배치하며 projection, rendering, persistence wiring을 갖지 않는다. 이미지 리소스 선택 시에만 `ImageEditor.qml`이 `ResourcesListModel.currentResourceEntry`를 받아 `source`/`resolvedPath`/`resourcePath` 기반 viewer로 동작한다. 포맷 단축키와 선택 텍스트 포맷 컨텍스트 메뉴는 바깥 `ContentViewLayout.qml` dispatch에서만 허용한다. C++이 계산한 문서 교체 뒤 커서 복원만 wrapper의 공개 LVRS cursor API를 통해 처리한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
