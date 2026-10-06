# `src/app/models/onboarding/IOnboardingHubController.hpp`

<a id="role"></a>

## 역할
`IOnboardingHubController`는 온보딩 경로 오케스트레이션에 필요한 작업 공간 전환 후크를 정의합니다.

<a id="contract"></a>

## 계약
- `beginWorkspaceTransition()`
- `completeWorkspaceTransition()`
- `failWorkspaceTransition(const QString&)`

<a id="notes"></a>

## 메모
- `OnboardingRouteBootstrapController`는 이제 구체적인 온보딩 컨트롤러 대신 이 인터페이스를 대상으로 합니다.
