# `src/extension/trial/WhatSonTrialRegisterXml.hpp`

<a id="role"></a>

## 역할
로컬 허브 등록 파일에 대한 평가판 전용 XML 리더 및 라이터를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `registerFileName()`: 고정 시험 레지스터 파일 이름 `trial_register.xml`를 반환합니다.
- `signatureAlgorithmName()`: 현재 XML 무결성 알고리즘 레이블을 반환합니다.
- `registerFilePath(...)`: 로컬 `.wshub` 루트 아래에서 `.whatson/trial_register.xml`를 확인합니다.
- `exists(...)`: 로컬 허브에 이미 평가판 레지스터 파일이 있는지 여부를 보고합니다.
- `loadRegister(...)`: XML 파일에서 정규화된 `deviceUUID` 및 `key` 쌍을 읽고 무결성 서명이 확인되었는지 보고합니다.
- `writeRegister(...)`: 정규화된 클라이언트 ID를 HMAC 서명과 함께 XML 파일에 저장합니다.

<a id="scope"></a>

## 범위
- 도우미는 로컬 허브 경로만 허용합니다.
- 제품 등록 파일 이름은 이 평가판 전용 모듈의 범위를 벗어납니다.
- 평가판 생성 코드는 `WhatSonTrialClientIdentityStore::ensureIdentity()`의 지속적인 ID를 `writeRegister(...)`에 제공할 것으로 예상됩니다.
- XML 서명은 로컬 등록 무결성 비밀과 비교하여 확인되므로 서명 확인은 현재 평가판 클라이언트 저장소에 따라 다릅니다.
