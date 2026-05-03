# `src/app/models/content`

## Responsibility
`src/app/models/content` owns non-editor content-surface helper objects that sit between app state and LVRS/QML hosts.

## Scope
- Mirrored source directory: `src/app/models/content`
- Child directories: 1
- Child files: 0

## Child Directories
- `mobile`

## Architectural Notes
- Editor-host display coordination now lives under `src/app/models/editor/display`.
- `mobile` owns mobile-only route planning, back-swipe, pop-repair, and hierarchy-selection preservation helpers that
  keep compact workspace navigation deterministic across note/detail/editor transitions.
- Structured editor host and policy types now live under `src/app/models/editor/structure`.

## Verification Notes
- Mobile content helpers are covered by the shared `whatson_cpp_regression` suite, including route-state restore and
  sidebar-binding resolution regression checks.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/content`` (`docs/src/app/models/content/README.md`)
- 위치: `docs/src/app/models/content`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
