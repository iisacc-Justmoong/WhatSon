# WhatSon 제품 개요 (Draft)

## 제품 정의

WhatSon은 LVRS 기반 Qt Quick 애플리케이션이며, 파일시스템 중심 워크스페이스(`.wshub`)를 읽어 계층형 탐색, 노트 목록, 본문 편집, 런타임 동기화를 수행하는 데스크톱/모바일 지향 제품이다.

## 현재 구현 범위

- Implemented: 다중 도메인 계층 뷰모델(`library/projects/bookmarks/tags/resources/progress/event/preset`)
- Implemented: `.wshub` 자동 탐색 후 병렬 로딩 및 메인 스레드 적용
- Implemented: 사이드바 계층 상호작용(선택, rename, drag/drop before/after/child/root)
- Implemented: 노트 목록-편집기 동기화 및 debounce 저장
- Implemented: 시스템 캘린더 포맷 연동
- Partial: 충돌(sync conflict) 표시 계약은 존재하나 생산자 경로는 미완
- Partial: daemon은 헬스체크 중심의 스켈레톤

## 비목표(현재 기준)

- 현재 버전은 네트워크 기반 중앙 서버 동기화 제품이 아니다.
- 동기화의 기본 단위는 `.wshub` 파일 패키지이며, 외부 클라우드/파일 제공자는 운용층에서 선택한다.

## 대상 사용자 관점 가치

- 계층+노트+편집을 하나의 일관된 워크스페이스로 운영
- 폴더/태그/북마크/프로젝트 등 다중 정보축을 동일 UI 패턴으로 처리
- 로컬 파일 중심 운용으로 소유권과 이식성 확보
