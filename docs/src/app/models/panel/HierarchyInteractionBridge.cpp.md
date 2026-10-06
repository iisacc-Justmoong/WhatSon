# `src/app/models/panel/HierarchyInteractionBridge.cpp`

<a id="role"></a>

## 역할
이 구현은 라이브 계층 구조 컨트롤러를 상호 작용 브리지에 바인딩하고 QML에 대해 캐시된 기능 상태를 유지합니다.

<a id="core-behavior"></a>

## 핵심 행동
- `setHierarchyController(...)` 는 `View -> Controller` 의존성 에지를 `verifyDependencyAllowed(...)` 를 통해 확인합니다. 브릿지가 QML 에서 생성된 런타임 어댑터이므로 루트 C++ 객체 그래프가 동결된 후에도 활성 계층 구조 컨트롤러가 재결합될 수 있기 때문에 의도적으로 변형 가능한 와이어링 잠금 게이트를 사용하지 않습니다.
- 브리지는 캐스트 `IHierarchyController*`를 `QPointer`로 저장합니다.
- `hierarchySelectionChanged` 및 `hierarchyNodesChanged`를 수신합니다.
- 모든 관련 변경 사항에서 캐시된 기능 부울을 다시 계산합니다.

<a id="capability-resolution"></a>

## 능력 해결
기능 점검은 `qobject_cast`를 통해 수행됩니다.
- 이름 바꾸기 지원은 `IHierarchyRenameCapability`에서 제공됩니다.
- 생성/삭제/보기 옵션 지원은 `IHierarchyCrudCapability`에서 제공됩니다.
- 확장 지원은 `IHierarchyExpansionCapability`에서 제공됩니다.

캐시된 속성 접근 방식은 QML가 JavaScript의 기능을 반복적으로 검색하는 대신 안정적인 부울 속성에 바인딩할 수 있기 때문에 중요합니다.

<a id="bulk-expansion-path"></a>

## 대량 확장 경로

- `setAllItemsExpanded(...)`는 먼저 구체적인 컨트롤러 `Q_INVOKABLE`에 대한 직접적인 Meta Object 호출을 시도합니다.
- 해당 대량 진입점을 사용할 수 없는 경우, 브리지는 행당 `IHierarchyExpansionCapability` 계약으로 되돌아가며, 직렬화된 계층 노드가 `showChevron: true`를 노출하는 행만 반복합니다.
- 이를 통해 QML 바닥글 메뉴를 작게 유지하면서 개별 도메인이 나중에 최적화된 대량 경로를 제공할 수 있습니다.

<a id="failure-model"></a>

## 실패 모델
컨트롤러가 없거나 기능을 구현하지 않거나 기능이 비활성화된 경우 브리지 안전하게 거부한다.
- 부울 쿼리는 `false`를 반환합니다.
- 명령이 작동하지 않게 됨

이는 QML 측을 더 단순하게 만들고 도메인이 기능을 지원하지 않을 때 부분적인 돌연변이 동작을 방지합니다.
