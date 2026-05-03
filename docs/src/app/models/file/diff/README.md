# `src/app/models/file/diff`

## Responsibility
Owns note snapshot diff/version logic centered on `.wsnversion` persistence and snapshot comparison.

## Scope
- Source directory: `src/app/models/file/diff`
- Child directories: none
- Child files: 2

## Child Files
- `WhatSonLocalNoteVersionStore.hpp`
- `WhatSonLocalNoteVersionStore.cpp`

## Architectural Notes
- Diff/version code was consolidated from `src/app/models/file/note` into this domain.
- `file/note/WhatSonLocalNoteFileStore` remains the note package orchestrator and delegates version snapshot/diff
  persistence to this module.

## Dependency Direction
- Depends on note document/header/body types under `src/app/models/file/note`.
- Consumed by note store update flow for capture/diff/checkout/rollback operations.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/file/diff`` (`docs/src/app/models/file/diff/README.md`)
- 위치: `docs/src/app/models/file/diff`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
