# `src/app/permissions/WhatSonPermissionBootstrapper.hpp`

<a id="responsibility"></a>

## 책임
앱 실행 배선에 사용되는 시작 권한 부트스트랩 코디네이터를 선언합니다.

<a id="public-surface"></a>

## 공공 표면
- `WhatSonPermissionBootstrapper(QCoreApplication& app)`: 앱 인스턴스에 바인딩하고 요청 단계를 사전 빌드합니다.
- `start()`: 순차적 권한 처리를 시작합니다.

<a id="internal-contract"></a>

## 내부 계약
- 권한 단계를 `PermissionStep{id, request}`로 저장하고 순서대로 실행합니다.
- `permissions/<id>/*` 아래의 `QSettings`에서 요청/승인 결정을 유지합니다.
- 다음을 모두 지원합니다.
  - Qt 권한 API 흐름(`addQtPermissionStep(...)`)
  - Apple 브리지 흐름(`addApplePermissionStep(...)`)

<a id="ordering-policy"></a>

## 주문 정책
- 빌드 순서에는 다음이 포함됩니다(플랫폼에 따라 다름).
  - 전체 디스크 액세스
  - 사진 라이브러리
  - 마이크
  - 접근성
  - 달력
  - 알림
  - 로컬 네트워크
  - 위치

<a id="completion-rule"></a>

## 완료 규칙
- 모든 단계가 끝나면 집계된 부여 상태를 `permissions/granted`에 기록합니다.
