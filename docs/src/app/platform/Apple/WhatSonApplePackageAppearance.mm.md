# `src/app/platform/Apple/WhatSonApplePackageAppearance.mm`

<a id="role"></a>

## 역할
`.wshub` 디렉터리에 대한 Apple 네이티브 패키지 프리젠테이션 패스를 구현합니다.

<a id="behavior"></a>

## 행동
- 들어오는 경로가 실제 디렉터리인지 확인합니다.
- 디렉터리에 대해 `NSURL`를 빌드하고 `NSURLIsPackageKey`를 `YES`로 설정합니다.
- Foundation이 패키지 힌트를 적용할 수 없는 경우 설명 오류를 반환합니다.

<a id="integration"></a>

## 통합
- `WhatSonHubPackager`를 통해서만 호출됩니다.
- 이제 스캐폴드 쓰기에 초점을 맞춘 `WhatSonHubCreator`에서 Apple 관련 패키지 동작을 유지합니다.
