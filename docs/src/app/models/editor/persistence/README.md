# `src/app/models/editor/persistence`

## Responsibility
Editor-owned persistence orchestration that preserves RAW `.wsnote/.wsnbody` as the only write authority.
This directory owns the note editor save boundary after a live editor session has produced a RAW body snapshot.

## Boundary
- `ContentsEditorPersistenceController` owns editor-session snapshot buffering, immediate enqueue attempts,
  retry/drain scheduling, pending-snapshot adoption, selected-note body reads, and post-save reconcile handoff.
- The controller may call `src/app/models/file/note/ContentsNoteManagementCoordinator` for concrete `.wsnote` IO, but
  file/note code must not decide editor-session save timing or keep editor-owned dirty buffers.
- `ContentsEditorSessionController` decides when the live session should stage or flush RAW text.
- `ContentsEditorSelectionBridge` remains the QML-facing adapter and delegates persistence work into this directory.
- Every helper here must mutate RAW source first, then let parser-derived projections update the UI.

## Current Files
- `ContentsEditorPersistenceController.hpp`
- `ContentsEditorPersistenceController.cpp`

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/persistence`` (`docs/src/app/models/editor/persistence/README.md`)
- 위치: `docs/src/app/models/editor/persistence`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
