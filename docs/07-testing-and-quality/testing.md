# 테스트와 품질 게이트 (Draft)

## 현재 확인된 테스트 축

- 파일/허브 런타임 스토어 검증
- 계층 ViewModel 계약 검증
- QML 바인딩/구문 가드
- SOLID 아키텍처 계약 테스트

## 품질 원칙

- 구현 안정성은 동작 테스트 + 계약 테스트의 이중 잠금으로 관리한다.
- UI 구조는 단순 스냅샷보다 계약 검증(필수 경계/필수 객체/금지 패턴) 비중을 높인다.

## 최근 반영 맥락

- hierarchy 엔진 관련 LVRS 측 신규 테스트(`hierarchy_codec`, `hierarchy_interaction`)가 통과된 상태이다.
- WhatSon 본체 문서에서는 이를 외부 인프라 연동 참고로만 기록하며, 제품 기능 기준 테스트와 분리해 관리한다.

## TODO

- 테스트 분류표(Unit/Contract/Integration/Smoke) 표준화
- 실패 시 triage 가이드(우선순위/소유 모듈) 추가
