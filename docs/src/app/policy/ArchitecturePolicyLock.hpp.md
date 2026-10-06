# `src/app/policy/ArchitecturePolicyLock.hpp`

<a id="role"></a>

## 역할
이 헤더는 애플리케이션 계층 종속성 제어에 대한 공개 어휘를 정의합니다.

그것은 2 개의 별개 개념을 노출합니다.
- `Layer` 열거형 및 도우미 함수를 통해 표현된 정적 종속성 정책입니다.
- `ArchitecturePolicyLock`를 통해 표현되는 런타임 잠금은 루트 개체 그래프가 완료되면 시작 시간 배선을 고정하는 데 사용됩니다.

<a id="public-api"></a>

## 공개 API
- `enum class Layer`: 런타임 종속성 확인 전반에 사용되는 정식 레이어 이름입니다.
- `layerName(Layer)`: 로그 및 진단에 사용되는 문자열 형식입니다.
- `isDependencyAllowed(Layer from, Layer to)`: 부작용이 없는 순수 정책 쿼리입니다.
- `assertDependencyAllowed(...)`: 부울을 반환하고 선택적으로 오류 메시지 형식을 지정합니다.
- `verifyDependencyAllowed(...)`: 동일한 규칙 확인이지만 잘못된 에지가 시도될 때 경고를 내보내야 하는 프로덕션 호출 사이트를 위한 것입니다.
- `verifyMutableWiringAllowed(...)`: 사후 잠금 런타임 재배선 시도를 거부하고 발신자 측 이유를 형식화할 수 있습니다.
- `verifyMutableDependencyAllowed(...)`: 하나의 공유 도우미에 잠금 후 변형 규칙과 레이어 종속성 규칙을 모두 적용합니다.
- `ArchitecturePolicyLock::isLocked()` 및 `ArchitecturePolicyLock::lock()`: 글로벌 시작 정지.
- `ArchitecturePolicyLock::unlockForTests()`: 회귀 케이스 간의 프로세스 전역 잠금 상태를 격리하는 데 사용되는 테스트 전용 탈출구입니다.

<a id="usage-pattern"></a>

## 사용 패턴
일반적인 소비자는 QML 방향 뷰를 QObject 기반 컨트롤러 또는 저장소에 연결하는 시작 시간 구성 코드 및 브리지와 같은 개체입니다.

예상되는 순서는 이렇습니다.
1. 개체 그래프를 작성합니다.
2. 필요한 협력자를 주입합니다.
3. `ArchitecturePolicyLock::lock()`에 전화하세요.
4. 이후의 구조적 재배선 시도를 거부합니다.

<a id="important-constraint"></a>

## 중요한 제약
이 헤더는 작은 정책 언어만 정의합니다. 아키텍처 자체를 자동으로 다시 작성하지 않습니다. 런타임 적용을 원하는 경우 프로덕션 코드는 확인 도우미를 호출해야 합니다.

현재 런타임 계약은 폴더 기반이 아닌 역할 기반입니다. 브리지/QML 방향 설정자 경로는 `View -> Controller`로 처리되고 컨트롤러 저장소 연결 경로는 `Controller -> Store`로 처리됩니다.
