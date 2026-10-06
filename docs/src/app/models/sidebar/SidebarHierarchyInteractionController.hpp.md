# `src/app/models/sidebar/SidebarHierarchyInteractionController.hpp`

<a id="role"></a>

## 역할
`SidebarHierarchyInteractionController`는 시각적 레이아웃 외의 계층 상호작용 정책을 위해 `SidebarHierarchyView.qml`가 사용하는 C++ 계약을 노출한다. 해당 유형은 생성 가능한 내부 QML QObject로 등록되므로 C++ 클래스는 `final`로 선언해서는 안 된다. Qt의 QML 등록 계층이 여기서 내부 `QQmlElement<T>` 래퍼를 파생하기 때문이다.

<a id="public-surface"></a>

## 공공 표면
- 이 클래스는 내부 QML 레지스트라가 이를 생성 가능한 QObject 타입으로 노출하고, Qt가 생성 가능한 타입을 `QQmlElement<T>`로 감싸기 때문에 의도적으로 `final`로 표시되지 않았습니다.
- `hierarchyInteractionBridge`는 컨트롤러를 기존 계층 구조 CRUD/확장 브리지에 바인딩합니다.
- `activeHierarchyIndex`는 활성 사이드바 도메인별로 확장 키를 보존합니다.
- `captureExpansionState(...)`, `modelWithPreservedExpansion(...)`, `handleExpansionSignal(...)`, `armExpansionForItem(...)`, `requestChevronExpansionForItem(...)` 및 `requestChevronExpansion(...)`는 QML에서 행 확장 상태, 스테이블키 파생 및 돌연변이 라우팅을 유지합니다.
- `beginActivationAttempt(...)`, `activationAttemptCurrent(...)` 및 `shouldSuppressActivation(...)`는 억제 정책을 소유하지 않고도 체브론 상호작용 후 뷰가 안전하게 활성화를 연기하도록 합니다.

<a id="signals"></a>

## 신호
컨트롤러는 확장 상태 및 바인딩 변경 신호만 내보냅니다. 바닥글 작업 신호 및 선택한 행 동기화 릴레이 신호는 의도적으로 없습니다. `SidebarHierarchyView.qml`는 바닥글 발송을 소유하고 `Qt.callLater(...)`와의 확장 후 시각적 동기화를 예약합니다.
