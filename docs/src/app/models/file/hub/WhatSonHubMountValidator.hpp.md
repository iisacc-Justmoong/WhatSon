# `src/app/models/file/hub/WhatSonHubMountValidator.hpp`

<a id="role"></a>

## 역할
`WhatSonHubMountValidator`는 마운트된 로컬 `.wshub` 패키지에 대한 지속형 또는 사용자 선택 허브 경로를 확인하고 해당 패키지가 런타임 부트스트랩를 입력할 수 있을 만큼 구조적으로 완전한지 여부를 보고합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- `resolveMountedHub(...)`는 원시 경로와 선택적 액세스 북마크를 허용합니다.
- 반환 페이로드는 발생하지 않고 성공적인 마운트를 유효하지 않거나 불완전한 허브 패키지와 구별합니다.
