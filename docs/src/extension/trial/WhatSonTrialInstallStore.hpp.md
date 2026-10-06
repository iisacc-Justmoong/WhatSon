# `src/extension/trial/WhatSonTrialInstallStore.hpp`

<a id="role"></a>

## 역할
선택적 평가판 확장에 대한 로컬 설치 날짜 키를 소유하는 지속성 도우미를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `defaultInstallDateSettingsKey()`: 평가판 모듈에서 사용하는 정식 `QSettings` 키를 반환합니다.
- `loadInstallDate()`: 유효한 날짜가 저장되지 않은 경우 지속된 날짜를 읽고 잘못된 `QDate`를 반환합니다.
- `ensureInstallDate(...)`: 처음 사용할 때 설치 날짜를 느리게 생성합니다.
- `storeInstallDate(...)`: ISO 형식으로 명시적인 설치 날짜를 기록합니다.
- `clear()`: 저장된 값을 제거합니다.

<a id="notes"></a>

## 메모
- 저장소는 보안 저장소 지원 레지스터 무결성 비밀을 사용하여 지속형 `QSettings` 페이로드에 서명합니다.
- 레거시 보안 저장소 설치 날짜는 계속해서 마이그레이션될 수 있지만 보안 저장소는 마이그레이션 후에 더 이상 미러 복사본으로 사용되지 않습니다.
- 소비자는 `QSettings`를 사용하기 전에 일반적인 Qt 조직 및 애플리케이션 이름을 설정해야 합니다.
