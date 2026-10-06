# `src/app/models/hierarchy/WhatSonHierarchyModel.cpp`

<a id="purpose"></a>

## 목적
- 모든 계층 항목 모델 표면을 지원하는 공유 `QAbstractListModel`를 구현합니다.
- UI를 향한 모델을 별도의 모델 클래스를 통해 각 도메인을 투사하는 대신 `LV.Hierarchy` 노드 맵에 가깝게 유지합니다.

<a id="behavior"></a>

## 행동
- `setItems(...)`는 삭제된 노드 맵을 저장하고 전체 노드 교체에 대해서만 재설정을 내보냅니다.
- `setItemExpanded(...)`는 한 행의 `expanded` 값을 변경하고 `ExpandedRole`에 대해 `dataChanged`를 내보냅니다.
- 검증은 의도적으로 일반화됩니다: 음수 깊이가 수정되고, 라벨이 트리밍되며, 깊이 0 이하의 악센트 행이 정규화됩니다.

<a id="testing"></a>

## 테스트
- `hierarchyItemModel_usesSharedLvrsModelContract`로 보호됩니다.
- 컨트롤러 채택은 `hierarchyControllers_exposeSharedLvrsHierarchyModel`에 의해 잠겨 있습니다.
