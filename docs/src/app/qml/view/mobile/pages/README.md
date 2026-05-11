# `src/app/qml/view/mobile/pages`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/mobile/pages`
- Child directories: 0
- Child files: 1

## Child Directories
- No child directories.

## Child Files
- `MobileHierarchyPage.qml`

## Recent Notes
- `MobileHierarchyPage.qml` is mounted by the adaptive/mobile `Main.qml` workspace route.
- The file keeps the routed hierarchy, note-list, editor, and detail shell. The editor route uses
  `ContentViewLayout.qml`, forwards `noteEditorSession`, and does not forward editor view-mode controller state.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/view/mobile/pages`` (`docs/src/app/qml/view/mobile/pages/README.md`)
- 위치: `docs/src/app/qml/view/mobile/pages`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 현재 workspace에서는 adaptive/mobile layout에서 `MobileHierarchyPage.qml`을 mount한다.
- editor route는 `ContentViewLayout.qml`을 사용하고 active note의 editor HTML session file을
  `LV.TextEditor`에 연결한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
