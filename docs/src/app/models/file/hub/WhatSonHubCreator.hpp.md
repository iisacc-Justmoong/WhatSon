# `src/app/models/file/hub/WhatSonHubCreator.hpp`

<a id="role"></a>

## 역할
사전 구체화된 `.wshub` 패키지 루트에 새 허브 스캐폴드를 작성하는 파일 시스템 지향 팩토리를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `createHub(...)`: 구성된 작업 공간 허브 루트 아래에 정리된 허브 패키지를 생성합니다.
- `createHubAtPath(...)`: 명시적 대상 경로에 허브 패키지를 생성하고 누락된 경우 `.wshub`를 추가합니다.
- `requiredRelativePaths(...)`: 파일 페이로드가 작성되기 전에 존재해야 하는 디렉터리 스캐폴드를 반환합니다.

<a id="creation-contract"></a>

## 생성 계약
- 성공적인 호출은 패키지 루트 구체화를 `WhatSonHubPackager`에 위임한 다음 `.whatson/hub.json`를 포함한 기본 스캐폴드를 작성합니다.
- 기존 허브 디렉터리는 하드 오류로 처리되며 절대 덮어쓰여지지 않습니다.
