# `src/app/models/file/hub/WhatSonHubPackager.hpp`

<a id="role"></a>

## 역할
`.wshub` 생성을 위한 패키지 루트 서비스를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `packageExtension()`: 정식 `.wshub` 접미사를 반환합니다.
- `normalizePackagePath(...)`: 요청된 허브 경로를 절대 `.wshub` 패키지 경로로 변환합니다.
- `createPackageRoot(...)`: 패키지 디렉터리를 생성하고 플랫폼별 패키지 표시 규칙을 적용합니다.

<a id="packaging-contract"></a>

## 포장계약
- 패키지 루트 생성은 허브 스캐폴드 생성과 별개입니다.
- 기존 패키지 디렉터리는 심각한 오류로 처리되며 절대 덮어쓰여지지 않습니다.
- Apple 특정 패키지 프레젠테이션은 `WhatSonHubCreator`에 포함되지 않고 이 경계 뒤에서 위임됩니다.
