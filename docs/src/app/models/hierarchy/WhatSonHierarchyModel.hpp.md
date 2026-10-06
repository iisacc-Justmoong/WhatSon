# `src/app/models/hierarchy/WhatSonHierarchyModel.hpp`

<a id="purpose"></a>

## 목적
- 모든 계층 도메인에서 사용되는 단일 LVRS 방향 계층 항목 모델을 선언합니다.
- 도메인 컨트롤러는 여전히 타입이 있는 변이 상태, 파서/스토어 및 선택 정책을 보유하고 있지만, 공통 `LV.Hierarchy` 행 계약에 대해 `WhatSonHierarchyModel* itemModel`를 노출합니다.

<a id="contract"></a>

## 계약
- `setItems(...)`는 각 컨트롤러의 `depthItems()`에서 이미 반환한 `QVariantList` 노드 형태를 허용합니다.
- 역할 이름은 `LV.Hierarchy`와 사이드바 브리지에서 사용되는 키를 그대로 반영합니다: `label`, `depth`, `expanded`, `showChevron`, `key`, `itemKey`, `iconName`, `count`, 드래그 플래그, 그리고 리소스/프로그레스 메타데이터.
- `setItemExpanded(...)`는 변경된 행에 대해 `ExpandedRole`만 업데이트하고 `dataChanged`를 방출하여 단일 체브론 접기/펼침 변경에 대해 전체 모델 리셋을 방지합니다.

<a id="notes"></a>

## 메모
- 도메인별 `*HierarchyModel.hpp` 파일은 이제 컨트롤러 내부에 대해 입력된 항목 구조체와 아이콘 도우미만 유지합니다.
- 새로운 계층 도메인은 다른 `QAbstractListModel` 서브클래스를 도입하는 대신 LVRS 노드 직렬화에서 이 모델을 공급해야 합니다.
