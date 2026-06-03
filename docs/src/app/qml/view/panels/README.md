# `src/app/qml/view/panels`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels`
- Child directories: 4
- Child files: 12

## Child Directories
- `detail`
- `list`
- `navigation`
- `sidebar`

## Child Files
- `BodyLayout.qml`
- `ContentViewLayout.qml`
- `DetailPanelLayout.qml`
- `HierarchySidebarLayout.qml`
- `ListBarHeader.qml`
- `ListBarLayout.qml`
- `NavigationBarLayout.qml`
- `NoteListItem.qml`
- `PanelEdgeSplitter.qml`
- `ResourceListItem.qml`
- `StatusBarLayout.qml`

## Recent Notes
- The current workspace route mounts the restored panel shell. `ContentViewLayout.qml` is the center content slot and
  now composes image resource viewing through `ImageEditor.qml` plus a blank placeholder for note and non-image
  selections. The deleted note editor toolbar and text surface must not be reconstructed in QML.
  kinetic carry for touch scrolling, and `SidebarHierarchyView.qml` explicitly pushes LVRS hierarchy scroll surfaces

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/panels`` (`docs/src/app/qml/view/panels/README.md`)
- 위치: `docs/src/app/qml/view/panels`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 현재 workspace route는 기존 panel shell을 mount하고, center content slot은 `ContentViewLayout.qml`을 통해
  이미지 리소스 viewer와 빈 content placeholder만 조합한다. 삭제된 note editor toolbar와 text surface는 QML에서
  다시 만들지 않는다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
