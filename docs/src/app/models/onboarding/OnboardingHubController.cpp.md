# `src/app/models/onboarding/OnboardingHubController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 생성자는 이제 `IOnboardingHubController` 베이스를 초기화합니다.
- 허브 선택, 생성 및 전환 동작이 이제 런타임 로드 콜백을 호출하기 전에 마운트/액세스 + 구조 검증을 `WhatSonHubMountValidator`에 위임합니다.
- 이러한 변경으로 인해 경로 오케스트레이션이 전체 구체적인 컨트롤러 표면에서 분리되었습니다.
