# `src/app/models/panel`

## Role
This directory contains QObject bridge types that translate generic or QML-facing panel behavior into narrow controller interactions.

These classes are not the core domain state. They are adaptation layers between:
- generic QML components
- dedicated hierarchy or content controllers
- architecture policy checks

## Important Bridge Types
- `HierarchyInteractionBridge`: rename, create, delete, and expansion access through capabilities.
- `HierarchyDragDropBridge`: reorder and note-drop access through capabilities.
- `FocusedNoteDeletionBridge`: focused-note deletion helper.
- `NoteListModelContractBridge`: dynamic note-list search/selection contract adapter used by `ListBarLayout.qml`.
- `PanelController` and `PanelControllerRegistry`: panel-specific controller routing and hook dispatch.

## Why This Layer Exists
Without these bridge objects, QML components would either:
- depend directly on large concrete controllers, or
- duplicate capability detection and guard logic in JavaScript.

The bridge layer keeps capability checks, ownership assumptions, and architecture-policy verification in one place.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/panel`` (`docs/src/app/models/panel/README.md`)
- 위치: `docs/src/app/models/panel`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
