# `src/app/register/WhatSonRegisterManager.cpp`

<a id="role"></a>

## 역할
`QSettings` 위에 지속형 인증 관리자를 구현합니다.

<a id="behavior"></a>

## 행동
- 기본 설정 키는 `register/authenticated`입니다.
- 생성은 지속된 플래그를 즉시 다시 로드하므로 새로운 관리자 인스턴스는 현재 앱 인증 상태를 반영합니다.
- 평가판 빌드는 인증된 우회 경로에 대해 더 이상 일반 부울을 신뢰하지 않습니다.
- 지속되는 평가판 빌드 값은 `register/authenticated`로 키가 지정되고 보안 저장소 지원 평가판 무결성 비밀에 대해 확인된 `QSettings`의 서명된 JSON 레코드입니다.
- 유효하지 않거나 형식이 잘못되었거나 서명되지 않은 레거시 값은 평가판 정책을 잠금 해제하는 대신 거부되고 지워집니다.
