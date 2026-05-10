# `src/app/qml/view/mobile`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/mobile`
- Child directories: 1
- Child files: 1

## Child Directories
- `pages`

## Child Files
- `MobilePageScaffold.qml`

## Recent Notes
- The restored workspace mounts the mobile scaffold for adaptive/mobile layouts.
- `pages/MobileHierarchyPage.qml` keeps the existing hierarchy, note-list, editor, and detail route shell, while the
  editor route forwards `NoteEditorDocumentSession` into `ContentViewLayout.qml` so the selected note's parsed RAW
  source session file is editable.
- The compact editor view-mode selector path remains removed.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/mobile`` (`docs/src/app/qml/view/mobile/README.md`)
- 위치: `docs/src/app/qml/view/mobile`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 현재 workspace에서는 adaptive/mobile layout에서 mobile scaffold를 mount한다.
- mobile editor route는 `ContentViewLayout.qml`을 통해 active note의 parsed RAW source session file을
  `LV.TextEditor`에 연결한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
