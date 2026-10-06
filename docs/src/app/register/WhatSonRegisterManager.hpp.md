# `src/app/register/WhatSonRegisterManager.hpp`

<a id="role"></a>

## 역할
지속적인 인증 완료 상태를 소유하는 앱 코어 등록 관리자를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `authenticated()`: 캐시된 인증 완료 플래그를 반환합니다.
- `setAuthenticated(...)`: 플래그를 업데이트하고 유지하며 값이 변경되면 `authenticatedChanged()`를 내보냅니다.
- `reload()`: `QSettings`에서 지속된 상태를 다시 로드하여 서명된 평가판 기록을 수락하기 전에 확인합니다.
- `clearAuthentication()`: 플래그를 `false`로 재설정하는 편의 슬롯입니다.

<a id="integration"></a>

## 통합
- `src/extension/trial`와 같은 선택적 모듈은 평가판 전용 코드를 필수 앱 빌드 그래프로 이동하지 않고도 이 관리자에 의존할 수 있습니다.
