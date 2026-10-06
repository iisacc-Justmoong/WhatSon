# `src/extension/trial/WhatSonTrialRegisterXml.cpp`

<a id="role"></a>

## 역할
선택적 평가판 확장을 위해 `.whatson/trial_register.xml` 직렬화를 구현합니다.

<a id="behavior"></a>

## 행동
- `<deviceUUID>`, `<key>` 및 `<signature>` 요소를 사용하여 작은 XML 페이로드를 작성합니다.
- 입력 ID를 유지하기 전에 정규화합니다.
- 로컬 레지스터 무결성 비밀을 사용하여 정식 페이로드에 대해 HMAC-SHA256 서명을 계산합니다.
- 부분적으로 작성된 레지스터 파일이 유효한 데이터에 대해 커밋되지 않도록 `QSaveFile`를 사용합니다.
- 형식이 잘못되었거나 불완전하거나 무결성이 일치하지 않는 레지스터 파일을 다시 로드할 때 거부합니다.
