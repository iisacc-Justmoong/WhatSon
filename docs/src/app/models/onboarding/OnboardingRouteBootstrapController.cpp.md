# `src/app/models/onboarding/OnboardingRouteBootstrapController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- `setHubController(...)`는 이제 `IOnboardingHubController`를 허용합니다.
- 경로 처리는 여전히 동일한 지점에서 `beginWorkspaceTransition`, `completeWorkspaceTransition` 및 `failWorkspaceTransition`를 호출합니다.
- 시작 구성은 이제 마운트 검증과 첫 번째 런타임 로드가 모두 성공한 후에만 허브를 작업공간 준비 상태로 간주하여, 마운트 사전 검사 이후에 발생하는 시작 실패에 대해 임베디드 온보딩을 확인할 수 있게 합니다.
