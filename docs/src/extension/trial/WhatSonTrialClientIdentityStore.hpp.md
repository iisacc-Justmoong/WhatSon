# `src/extension/trial/WhatSonTrialClientIdentityStore.hpp`

<a id="role"></a>

## 역할
현재 설치에 대한 지속형 장치 UUID, 32문자 클라이언트 키 및 등록 무결성 비밀을 소유하는 평가판 전용 인앱 ID 저장소를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `loadDeviceUuid()` / `loadClientKey()`: 정규화된 지속 값을 반환하거나 유효하지 않은 저장된 데이터를 지웁니다.
- `loadRegisterIntegritySecret()`: 현재 설치에 대해 정규화된 레지스터 서명 비밀을 반환합니다.
- `loadIdentity()`: 대체를 생성하지 않고 정규화된 값 쌍을 반환합니다.
- `ensureIdentity()`: 장치 UUID, 클라이언트 키 및 레지스터 무결성 비밀이 존재함을 보장합니다.
- `ensureRegisterIntegritySecret()`: `trial_register.xml`에 사용되는 HMAC 비밀을 느리게 생성합니다.
- `storeIdentity(...)`: 저장된 ID를 정규화된 값으로 바꿉니다.
- `clear()`: 보안 저장소에서 평가판 전용 ID 및 등록 무결성 비밀을 제거하고 나머지 레거시 일반 설정을 모두 지웁니다.

<a id="data-contract"></a>

## 데이터 계약
- `deviceUuid`는 클라이언트 키와 별도로 저장되므로 향후 평가판 흐름에서 XML를 구문 분석하지 않고도 검사할 수 있습니다.
- `key`는 항상 32문자 영숫자 문자열이어야 합니다.
- 레지스터 무결성 비밀은 64문자 소문자 16진수 문자열입니다.
