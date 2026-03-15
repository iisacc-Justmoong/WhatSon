# WhatSon Documentation (Draft)

본 문서는 현재 구현 상태를 기준으로 작성된 진행중 초안이다. 완결 문서가 아니며, 코드 변경에 따라 지속 갱신되어야 한다.

## 문서 범위

- 구현 확인 기반의 제품/개념/아키텍처/모듈/빌드/테스트 문서
- 미구현 또는 정책 검토가 필요한 항목은 TODO로 명시

## 문서 맵

- `01-product/overview.md`: 제품 개요와 현재 기능 범위
- `02-concepts/workspace-and-hub.md`: `.wshub` 중심 데이터/런타임 개념
- `03-architecture/runtime-bootstrap.md`: 앱 부트스트랩 및 런타임 로딩 흐름
- `04-modules/app/module-map.md`: app 모듈 구조와 책임
- `04-modules/daemon/module-map.md`: daemon 현재 상태
- `04-modules/cli/module-map.md`: rust-cli 설치/운영 진입점
- `06-build-and-release/build.md`: 현재 빌드/설치 절차
- `07-testing-and-quality/testing.md`: 테스트와 품질 게이트
- `10-roadmap/documentation-todo.md`: 문서 보완 로드맵

## 상태 표기

- `Implemented`: 코드에서 확인된 구현
- `Partial`: 일부 구현, 추가 정리가 필요한 상태
- `Planned`: 계획만 있고 구현/검증 미완료
