# `src/app/models/hierarchy/IHierarchyController.hpp`

<a id="role"></a>

## 역할
이 파일은 계층 구조 컨트롤러에 대한 공유 읽기 지향 QObject 계약을 정의합니다.

많은 소비자에게 다음이 필요하다는 내용으로 의도적으로 제한되었습니다.
- 계층 노드
- 선택된 인덱스
- 품목 수
- 로드 성공 상태
- 마지막 로드 오류
- 계층 항목 모델 및 메모 목록 모델에 대한 액세스

<a id="public-surface"></a>

## 공공 표면
`Q_PROPERTY` 레이어가 존재하므로 QML는 지원 도메인에 관계없이 정규화된 계층 구조에 바인딩할 수 있습니다.

필요한 가상 기능은 하위 수준 구현 계약에 매핑됩니다.
- `itemModel()`
- `noteListModel()`
- `selectedIndex()` 및 `setSelectedIndex(...)`
- `itemCount()`
- `loadSucceeded()`
- `lastLoadError()`
- `hierarchyModel()`
- `itemLabel(...)`

<a id="signal-bridging"></a>

## 신호 브리징
`initializeHierarchyInterfaceSignalBridge()`가 중요합니다. 구체적인 계층 구조 컨트롤러는 `selectedIndexChanged()` 및 `hierarchyModelChanged()`와 같은 도메인 로컬 신호를 노출할 것으로 예상됩니다. 이 도우미는 해당 구현 신호를 일반 QML에서 사용하는 정규화된 인터페이스 수준 신호로 다시 연결합니다.

<a id="shared-expansion-helpers"></a>

## 공유 확장 도우미
`setHierarchyItemExpanded(...)` 와 `setAllHierarchyItemsExpanded(...)` 는 구체적 계층 컨트롤러를 위한 보호된 부모 헬퍼입니다. 모든 계층이 공유하는 오른쪽 화살표 확장 규칙을 중앙집중화하며: 유효하지 않은 인덱스를 거부하고, `showChevron` 가 없는 행을 거부하며, 요청된 상태가 이미 현재 상태인 경우 무작정 작업을 수행하고, 메모리 내 항목 상태가 변경된 후에만 도메인의 커밋 후크를 호출합니다.

<a id="what-this-interface-no-longer-does"></a>

## 이 인터페이스가 더 이상 수행하지 않는 작업
이 인터페이스는 이제 이름 변경, 생성, 삭제, 재순서, 확장, 또는 노트 드롭 작업을 공개 뷰에 노출하는 API 로 더 이상 노출하지 않습니다. 이러한 동작은 전용 기능 인터페이스로 이동하여 읽기 전용 소비자가 쓰기 집약적 API 에 의존하지 않습니다. 보호된 확장 헬퍼는 자식 컨트롤러를 위한 구현 재사용이며, 공개 변형 표면이 아닙니다.

<a id="practical-effect"></a>

## 실질적인 효과
QML 표면이 계층 구조 상태만 표시하면 되는 경우 이 인터페이스에 바인딩하세요. 계층 구조 상태를 변경해야 하는 경우 브리지를 통해 이 인터페이스를 하나 이상의 기능 인터페이스와 결합합니다.
