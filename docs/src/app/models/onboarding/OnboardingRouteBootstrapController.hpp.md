# `src/app/models/onboarding/OnboardingRouteBootstrapController.hpp`

<a id="role"></a>

## 역할
`OnboardingRouteBootstrapController`는 내장된 온보딩 가시성과 경로 커밋을 구동합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 이제 컨트롤러는 `OnboardingHubController` 대신 `IOnboardingHubController`를 저장합니다.
- 이렇게 하면 라우트 로직이 전환 훅에만 바인딩되고 전체 온보딩 구현에는 바인딩되지 않습니다. 검증이나 첫 번째 런타임 로드가 앱을 사용할 수 있는 작업 공간 없이 남깁니다.
