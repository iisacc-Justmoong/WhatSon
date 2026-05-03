# `src/app/qml/view/panels/navigation`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/navigation`
- Child directories: 3
- Child files: 10

## Child Directories
- `control`
- `edit`
- `view`

## Child Files
- `NavigationAddNewBar.qml`
- `NavigationApplicationAddNewBar.qml`
- `NavigationApplicationCalendarBar.qml`
- `NavigationApplicationPreferenceBar.qml`
- `NavigationCalendarBar.qml`
- `NavigationEditorViewBar.qml`
- `NavigationInformationBar.qml`
- `NavigationModeBar.qml`
- `NavigationPreferenceBar.qml`
- `NavigationPropertiesBar.qml`

## Recent Notes
- The small shared navigation bars now use `LV.Theme.gap...` tokens for their inter-button spacing instead of local
  integer literals such as `2`, `4`, `8`, or `12`.
- `NavigationEditorViewBar.qml` also moved its popup offset and menu width to LVRS token/scale-aware metrics.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/panels/navigation`` (`docs/src/app/qml/view/panels/navigation/README.md`)
- 위치: `docs/src/app/qml/view/panels/navigation`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
