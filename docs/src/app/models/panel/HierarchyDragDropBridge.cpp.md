# `src/app/models/panel/HierarchyDragDropBridge.cpp`

<a id="role"></a>

## 역할
이 구현은 QML의 드래그/드롭 작업을 기능 기반 계층 구조 변형에 적용합니다.

<a id="core-behavior"></a>

## 핵심 행동
- `setHierarchyController(...)`는 정책 레이어를 통해 `View -> Controller` 엣지를 검증합니다. 아키텍처 잠금 후에도 QML가 뷰 런타임에서 이 브리지를 생성하고 이를 활성 계층 도메인으로 전환하기 때문에 재바인드가 유지됩니다.
- 브리지는 계층 구조 노드 변경과 선택 변경을 모두 감시합니다.
- `refreshContractState()`는 재정렬 및 메모 삭제가 가능한지 여부를 캐시합니다.
- `refreshSelectedItemKey()`는 정규화된 계층 모델에서 현재 선택된 노드 키를 추출합니다.

<a id="reorder-path"></a>

## 경로 재정렬
`applyHierarchyReorder(...)`는 다음과 같은 경우에만 성공합니다.
- 계층 구조 컨트롤러가 존재합니다
- 컨트롤러는 `IHierarchyReorderCapability`를 구현합니다.
- 재주문 지원이 활성화되었습니다
- 들어오는 노드 목록이 비어 있지 않습니다

들어오는 모델은 C++ `QVariantList` 또는 `LV.Hierarchy.model` 에서의 QML / JS 배열 변형일 수 있으며, 브릿지는 컨트롤러를 호출하기 전에 두 형식을 모두 정규화합니다. 브릿지는 사용 가능한 경우 호출자가 제공한 활성 항목 키를 전달하고 그렇지 않으면 현재 선택된 키로 되돌아갑니다.

`applyHierarchyMove(...)` 은 컨트롤러가 단일 가시 이동 재생을 원할 때 저수준 타겟 이동 보조 도구로 계속 사용 가능합니다. 사이드바의 일반 LVRS 드래그 경로는 `applyHierarchyReorder(...)` 를 사용하며, `LV.Hierarchy` 가 `onListItemMoved` 를 방출하기 전에 이미 가시 트리 이동을 모델에 적용했기 때문입니다.

<a id="note-drop-path"></a>

## 노트 드롭 경로
`canAcceptNoteDrop(...)` 및 `assignNoteToFolder(...)` 모두:
- `IHierarchyNoteDropCapability` 필요
- 노트 ID를 정규화하고 다듬습니다.
- 위임하기 전에 음수 인덱스와 빈 노트 ID를 거부합니다.

이렇게 하면 QML 레이어가 반복적인 정리 로직에서 벗어나도록 유지하고 잘못된 형식의 드래그 페이로드가 조기에 실패하도록 보장합니다.

노트 목록 다중 선택을 위해 `canAcceptNoteDropList(...)` 와 `assignNotesToFolder(...)` 는 QML 가 제공한 노트 ID 배열을 정규화/중복 제거하고 선택된 노트에 걸쳐 동일한 기능 계약을 재생합니다. `assignNotesToFolder(...)` 는 `SidebarHierarchyView` 의 폴더 행에 `ListBarLayout` 노트 목록 위임이 놓여질 때 브릿지 수준의 커밋이며, 구체적인 계층 구조 컨트롤러는 노트 헤더 폴더 바인딩을 작성하고 파생된 노트 목록을 새로 고치는 데 계속 책임이 있습니다.
