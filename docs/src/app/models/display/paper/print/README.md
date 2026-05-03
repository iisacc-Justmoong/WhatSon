# `src/app/models/display/paper/print`

## Responsibility
Owns print-specific paper layout and option helpers.

## Child Files
- `ContentsPagePrintLayoutRenderer.cpp`
- `ContentsPagePrintLayoutRenderer.hpp`

## Current Notes
- `ContentsPagePrintLayoutRenderer` centralizes page/print mode gating and print margin-guide calculations for every
  surface that needs paper-layout state.
- Canonical A4 geometry and paper background tokens are sourced from `src/app/models/display/paper/ContentsA4PaperBackground.*`
  so `print/` keeps the print-specific layout policy rather than owning the shared paper definition.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/display/paper/print`` (`docs/src/app/models/display/paper/print/README.md`)
- 위치: `docs/src/app/models/display/paper/print`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
