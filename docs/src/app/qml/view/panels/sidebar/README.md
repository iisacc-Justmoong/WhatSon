# `src/app/qml/view/panels/sidebar`

## Role
This directory contains the visual sidebar hierarchy surface and the domain-specific wrappers that mount it.

The directory is intentionally split between:
- one reusable visual host: `SidebarHierarchyView.qml`
- inline helper controllers for selection, rename, note-drop, and bookmark visuals inside `SidebarHierarchyView.qml`
- lightweight wrappers such as `HierarchyViewLibrary.qml`, `HierarchyViewProjects.qml`, and related domain panels

## Important Design Choice
`SidebarHierarchyView.qml` is not expected to keep every interaction detail in the root object. Recent cleanup removed
the old sibling helper files and keeps the controller-style responsibilities as named inline `QtObject` helpers:
- `hierarchySelectionController`
- `renameController`
- `noteDropController`
- `bookmarkPaletteController`

This keeps the root sidebar view focused on composition, geometry, and signal forwarding.

The rename path is intentionally split between `SidebarHierarchyView.qml` and
`renameController`: the controller owns the transaction, while the view owns the rendered
`displayedHierarchyModel` snapshot so inline rename can hide only the visible label without dropping stable row
identity.

Each inline helper must initialize its required root dependencies explicitly. If those bindings are missing, QML can
abort workspace route construction with required-property errors and show only the empty LVRS window background.

The sidebar also now owns an explicit mobile kinetic-scroll contract. `SidebarHierarchyView.qml`
does not just rely on LVRS defaults anymore; it pushes the shared `LV.Hierarchy` surface onto a
touch-friendly overshoot/flick profile so mobile hierarchy scrolling keeps momentum after release.

## Relationship To C++
This directory talks to C++ almost entirely through:
- `IHierarchyController`-compatible objects
- `HierarchyInteractionBridge`
- `HierarchyDragDropBridge`

That separation is what keeps domain-specific mutation logic out of the QML file itself.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/panels/sidebar`` (`docs/src/app/qml/view/panels/sidebar/README.md`)
- 위치: `docs/src/app/qml/view/panels/sidebar`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
- 현재 기준: 삭제된 sibling helper QML 파일 대신 `SidebarHierarchyView.qml` 내부 inline `QtObject` helper가
  선택, 이름 변경, 드롭, 북마크 팔레트 책임을 담당한다.
