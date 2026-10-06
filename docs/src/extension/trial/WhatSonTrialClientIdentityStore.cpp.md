# `src/extension/trial/WhatSonTrialClientIdentityStore.cpp`

<a id="role"></a>

## 역할
평가판 전용 클라이언트 ID 지속성 계층을 구현합니다.

<a id="behavior"></a>

## 행동
- 잘못 저장된 UUID, 키 또는 레지스터 비밀 값은 읽기 중에 제거됩니다.
- 장치 UUID, 클라이언트 키 및 등록 무결성 비밀은 이제 평가판 보안 저장소에만 있습니다.
- 레거시 일반 `QSettings` 값은 보안 저장소 백엔드를 사용할 수 있는 경우에만 마이그레이션 입력으로 처리된 다음 `QSettings`에서 제거됩니다.
- 누락된 장치 UUID 값은 사용 가능한 경우 `QSysInfo::machineUniqueId()`에서 임의의 UUID 대체 경로와 함께 파생됩니다.
- 누락된 클라이언트 키는 32문자 영숫자 문자열로 생성됩니다.
- 누락된 레지스터 무결성 비밀은 64문자 소문자 16진수 문자열로 생성됩니다.
- `ensureIdentity()`는 생성된 값을 보안 저장소에 유지하고 성공을 보고하기 전에 다시 로드합니다.
