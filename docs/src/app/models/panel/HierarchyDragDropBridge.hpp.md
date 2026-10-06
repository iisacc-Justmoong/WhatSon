# `src/app/models/panel/HierarchyDragDropBridge.hpp`

<a id="role"></a>

## 역할
이 헤더는 드래그/드롭 관련 작업을 위해 QML 계층 구조 보기에서 사용되는 브리지를 정의합니다.

그것은 3 아이디어를 QML 에 노출합니다.
- 재정렬 지원 여부.
- 노트 드롭 지정이 지원되는지 여부입니다.
- 현재 선택된 계층 구조 항목 키입니다.

<a id="public-surface"></a>

## 공공 표면
- 속성:
  - `hierarchyController`
  - `reorderContractAvailable`
  - `noteDropContractAvailable`
  - `selectedItemKey`
- 호출 가능 항목:
  - `applyHierarchyReorder(...)` , C++ `QVariantList` 또는 QML 가 LVRS 를 적용한 후 반환한 JS -형식의 `var` 모델을 받아들이는.
  - `applyHierarchyMove(...)`, LVRS `listItemMoved` 이벤트 페이로드 ( `fromIndex`, `toIndex`, `depth` 및 활성 항목 키) 를 직접 활성 재순환 가능 컨트롤러로 전달합니다.
  - `canAcceptNoteDrop(...)`
  - `canAcceptNoteDropList(...)`
  - `assignNoteToFolder(...)`
  - `assignNotesToFolder(...)`

<a id="design-constraint"></a>

## 설계 제약
이 브리지는 일반 계층 인터페이스와 기능 인터페이스만 이해합니다. 도메인별 정책을 확장해서는 안 됩니다. 도메인별 승인/거부 논리는 구체적인 계층 컨트롤러 기능 구현에 속합니다.
