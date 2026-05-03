# `src/app/models/display/paper`

## Responsibility
Owns paper-surface helpers shared by page and print editor modes.

## Child Files
- `ContentsA4PaperBackground.cpp`
- `ContentsA4PaperBackground.hpp`
- `ContentsPaperSelection.cpp`
- `ContentsPaperSelection.hpp`

## Child Directories
- `print`

## Current Notes
- `ContentsA4PaperBackground` is the canonical A4 paper background object for shared paper geometry and palette tokens.
- `ContentsPaperSelection` is the shared enum-backed paper-choice object that tells the rest of the display layer
  which paper standard is currently selected.
- Inline text formatting moved to `src/app/models/editor/format`, so this directory only owns reusable paper-surface
  state and the `print` child shard owns print layout.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/display/paper`` (`docs/src/app/models/display/paper/README.md`)
- 위치: `docs/src/app/models/display/paper`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
