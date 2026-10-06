# `src/app/models/onboarding/OnboardingHubController.hpp`

<a id="role"></a>

## 역할
`OnboardingHubController`는 온보딩 상태 변형, 허브 생성/로딩 콜백 및 작업 공간 전환 상태를 소유합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 경로 계층 통신을 위해 `IOnboardingHubController`를 구현합니다.
- QML에 사용할 수 있는 더 넓은 온보딩 표면, 속성 및 신호를 유지합니다.
- 내부 허브 탑재 검증은 `src/app/models/file/hub`의 공유 허브 검증기에 위임됩니다.
