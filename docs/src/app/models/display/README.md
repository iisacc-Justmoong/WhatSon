# `src/app/models/display`

## Responsibility
Owns display-mode model helpers that sit below QML and above raw editor/controller consumers.

## Current Domains
- `paper`: canonical A4 paper background definitions plus common paper-surface presentation helpers shared by page and
  print view modes, including the shared paper-selection enum object.

## Architectural Note
- Display-mode objects belong under `models` because they expose stable QObject state and calculations for the UI, but
  they are not themselves QML views and should not live in the editor renderer tree once they become reusable
  cross-surface mode helpers.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/display`` (`docs/src/app/models/display/README.md`)
- 위치: `docs/src/app/models/display`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
