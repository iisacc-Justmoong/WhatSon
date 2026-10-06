# `src/app/models/sidebar/SidebarHierarchyController.hpp`

<a id="role"></a>

## 역할
`SidebarHierarchyController`는 사이드바 선택 상태와 활성 계층 바인딩을 소유합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 이제 `IActiveHierarchyContextSource`를 구현합니다.
- 시작/ 런타임 조정은 전체 사이드바 구현 유형에 의존하지 않고도 상속된 `IActiveHierarchySource` 계약을 통해 여전히 활성화를 관찰할 수 있습니다.
- 부유한 소비자는 이제 `SidebarHierarchyController`를 직접 명명하지 않고도 액티브 계층 구조 컨트롤러와 액티브 노트리스트 모델을 읽기 위해 컨텍스트 수준 인터페이스에 의존할 수 있습니다.
- 생성자는 더 이상 QObject 양육을 위한 직접 기반 초기화자 이름을 하드코딩하지 않습니다. 이제 생성자 본문에서 부모 소유가 `setParent(parent)`를 통해 적용되므로, 인터페이스 레이어 리팩터링은 클래스를 컴파일 상태로 유지하기 위해 초기화자 목록에 대한 두 번째 수동 업데이트가 필요하지 않습니다.
- `activeHierarchyIndex() const noexcept`는 이제 명시적으로 `override` 로 표시되며, 선언을 순수 가상 인터페이스 계약에 맞게 정렬하고 생성된 moc 빌드 중에 컴파일러 경고 소음을 제거합니다.
- 컨트롤러는 또한 `activeBindingsChanged()`를 발행합니다. 이는 합성 변경 신호로, 활성 계층 인덱스, 활성 계층 제어기 및 활성 노트 리스트 모델을 하나씩 반응하는 대신 하나의 일관된 스냅샷으로 갱신해야 하는 소비자에게 적용됩니다.
