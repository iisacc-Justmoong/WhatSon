# `src/app/platform/Apple/WhatSonApplePackageAppearance.hpp`

<a id="role"></a>

## 역할
`.wshub` 디렉터리를 패키지로 표시하는 Apple 네이티브 도우미를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `applyPackageDirectoryPresentation(...)` : 이미 생성된 디렉터리 경로에 Apple 패키지 프레젠테이션 힌트를 적용합니다.

<a id="boundary"></a>

## 경계
- 이 헬퍼는 `platform/Apple` 아래에 의도적으로 격리되어 있어 `WhatSonHubPackager`가 휴대용 C++에 머무르면서도 Finder/문서 브라우저 패키지 동작을 Objective- C++에 위임할 수 있습니다.
