# Documentation Mirror

This directory mirrors the maintainable `src` tree under `docs/src/...`.

The mirror has two goals.
- Preserve a one-directory/one-file correspondence with the live codebase.
- Allow the repository to be documented incrementally without inventing a separate documentation taxonomy.

## Current State
- The full `src` tree has been mirrored into `docs/src/...`.
- Every maintained source directory has a `README.md`.
- Every maintained source file has a sibling `.md` document.
- A first detailed pass now exists for the core startup, model-domain runtime contract, hierarchy bridge, and sidebar composition areas.
- The remaining files still contain scaffold text and must be upgraded in later passes.

## Generation Rules
- Mirrors stable source directories under `docs/src/...`.
- Excludes hidden files, `.DS_Store`, and generated Rust build output under `src/cli/target`.
- Uses `README.md` as the directory entry point.
- Uses `<source-file-name>.md` for file-level documents.

## Recommended Reading Order
1. `docs/src/app/README.md`
2. `docs/src/app/main.cpp.md`
3. `docs/src/app/qml/Main.qml.md`
4. `docs/src/app/models/file/hierarchy/README.md`
5. `docs/src/app/models/panel/README.md`
6. `docs/src/app/qml/view/panels/sidebar/README.md`

## Authoring Rule
- Keep documentation grounded in the live implementation.
- Prefer naming concrete classes, signals, QML objects, and ownership paths over vague summaries.
- When architecture changes, update both the directory README and the affected file documents.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `Documentation Mirror` (`docs/README.md`)
- 위치: `docs`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
