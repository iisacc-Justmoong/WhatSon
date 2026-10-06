# `src/app/store/hub/ISelectedHubStore.hpp`

<a id="role"></a>

## 역할
`ISelectedHubStore`는 지속형 스타트업 허브 선택 계약을 정의합니다.

<a id="contract"></a>

## 계약
- 현재 허브 경로를 읽고 북마크에 액세스합니다.
- 지속된 선택을 지우거나 업데이트합니다.

<a id="notes"></a>

## 메모
- 시작 해결 코드는 지속적인 선택을 위해서만 이 인터페이스에 따라 달라집니다.
- Blueprint 대체 경로 정책은 이제 설정 지원 저장소가 아닌 `WhatSonStartupHubResolver`에 있습니다.
