# `src/app/qml/view/panels/sidebar/SidebarHierarchyNoteDropController.qml`

<a id="responsibility"></a>

## 책임

`SidebarHierarchyNoteDropController.qml`는 계층 구조 사이드바에 대한 메모-폴더 드롭 디코딩을 중앙 집중화합니다.

<a id="behavior"></a>

## 행동

- 원시 포인터 좌표에서 마우스로 가리킨 계층 구조 행을 확인하고 이를 정규화된 놓기 대상으로 노출합니다.
- `noteDropIndexAtPosition(...)`는 동일한 대상 리졸버에 위임하고 해결된 대상 인덱스를 반환해야 하므로, 호출자는 그렇지 않으면 유효한 노트 드롭 히트 테스트에서 `undefined`를 절대 받지 않습니다.
- 드래그 페이로드를 고유한 노트 ID 배열로 정규화하고, `ListBarLayout.qml`에서 내보낸 다중 선택 페이로드를 포함합니다.
- `normalizeNoteIds(...)`는 항상 중복 제거된 배열을 반환해야 합니다; 반환이 누락될 경우 폴더 드롭 수락이 빈 페이로드로 축소되고 메모 할당이 완전히 차단됩니다.
- `collectHierarchyItems()`는 항상 발견된 계층 행 목록을 반환해야 합니다. 이는 형제 사이드바 헬퍼가 호버, 팔레트 및 드롭 인접 동작에 대해 동일한 아이템 로케이터 계약을 재사용하기 때문입니다.
- 가능한 경우 `HierarchyDragDropBridge.canAcceptNoteDropList(...)` / `assignNotesToFolder(...)`를 사용하고, 구형 기능 표면에는 단일 노트 대체 경로를 사용합니다.
- 페이로드가 비어 있거나 호버된 폴더가 드래그된 메모를 전혀 받아들일 수 없을 때 호버 미리보기 상태를 초기화합니다. 외부 `DropArea.enabled` 바인딩이 아니라 컨트롤러가 이 대상 수준의 수락/거부 결정을 소유합니다.
- 외부 계층 구조 드롭 표면은 여전히 `whatson.library.note` 드래그 키로 제한됩니다. 계층 구조 아이템 드래그는 노트 드롭 표면 외부에 있어야 하며, 따라서 LVRS는 자체 `listItemMoved` 신호를 통해 트리 순서를 재배열할 수 있습니다.

<a id="regression-notes"></a>

## 회귀 노트

- `test/cpp/suites/sidebar_hierarchy_rename_controller_tests.cpp`는 노트 드롭 표면 계약을 잠그어 `DropArea`가 노트 드래그를 위해 열려 있는 상태를 유지하고, 빈 페이로드를 거부하며, 계층 항목 재배열 드래그를 가로채지 않도록 합니다.
- 회귀 체크리스트:
    - JSON `application/x-whatson-note-ids` 페이로드는 고유한 순서의 메모 ID 목록으로 디코딩되어야 합니다.
    - 일반 텍스트 줄바꿈으로 구분된 페이로드는 여전히 동일한 메모 ID 세트로 디코딩되어야 합니다.
    - 다중 선택된 노트 목록 드래그는 적어도 하나의 드래그된 노트가 폴더에 할당될 수 있을 때 폴더를 강조 표시해야 합니다.
    - 해당 페이로드를 삭제하면 할당 가능한 모든 드래그 노트가 계층 드래그/드롭 브리지를 통해 라우팅되어야 합니다.
    - `noteDropIndexAtPosition(...)`는 `undefined`에 속하지 않고 확인된 대상 인덱스를 반환해야 합니다.
    - `normalizeNoteIds(...)`와 `collectHierarchyItems()`는 명시적인 `return normalized;` / `return items;` 구문을 유지해야 하므로 QML가 아무런 알림 없이가 `undefined`를 생성하지 않도록 해야 합니다.
