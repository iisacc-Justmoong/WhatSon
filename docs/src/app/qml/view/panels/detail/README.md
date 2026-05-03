# `src/app/qml/view/panels/detail`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/detail`
- Child directories: 0
- Child files: 10

## Child Directories
- No child directories.

## Child Files
- `DetailContents.qml`
- `DetailFileStatForm.qml`
- `DetailMetadataHierarchyPicker.qml`
- `DetailMetadataSelectionController.qml`
- `DetailPanel.qml`
- `DetailPanelHeaderToolbar.qml`
- `DetailPanelHeaderToolbarButton.qml`
- `NoteDetailPanel.qml`
- `ResourceDetailPanel.qml`
- `RightPanel.qml`

## Recent Notes
- `DetailPanel.qml` is now only the hierarchy-aware router for the detail column, and it switches between
  `NoteDetailPanel.qml` and `ResourceDetailPanel.qml` instead of forcing one note-only form onto every hierarchy.
- `ResourceDetailPanel.qml` is intentionally blank for now, but it already owns a dedicated resource-detail
  controller contract so future resource-only UI can grow without reopening note-detail branching.
- `DetailContents.qml` is shared by desktop and mobile detail routes, so compact section spacing and fixed-height
  surfaces must remain LVRS scale-aware instead of assuming desktop `1.0x` metrics.
- `DetailMetadataHierarchyPicker.qml` now owns the shared folder/tag add overlay so desktop and mobile reuse the
  same hierarchy rendering contract through an overridden `LV.ContextMenu`, and the picker body now renders the full
  hierarchy as a permanently expanded `LV.HierarchyItem` list instead of an embeddable tree panel.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/panels/detail`` (`docs/src/app/qml/view/panels/detail/README.md`)
- 위치: `docs/src/app/qml/view/panels/detail`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
