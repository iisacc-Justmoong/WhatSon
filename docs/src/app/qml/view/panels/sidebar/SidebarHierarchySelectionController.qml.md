# `src/app/qml/view/panels/sidebar/SidebarHierarchySelectionController.qml`

<a id="responsibility"></a>

## 책임

`SidebarHierarchySelectionController.qml`는 `SidebarHierarchyView.qml`에 대한 계층 다중 선택 동작을 소유합니다.

포인터 수정자 캡처, 범위/토글 선택 의미 체계 및 기본 행 활성화를 중앙 집중화하므로 루트 사이드바 보기가 LVRS 구성 및 오버레이 렌더링에 계속 집중할 수 있습니다.

<a id="public-contract"></a>

## 공공 계약

- `selectionAnchorIndex`: 계층 범위 선택 앵커.
- `selectedIndices`: 시각적으로 선택된 계층 구조 행의 정규화된 집합입니다.
- `pointerSelectionModifiers` / `pointerSelectionModifiersCapturedAtMs`: 수정자 복구를 위한 단기 프레스 캐시.
- `requestHierarchySelection(item, resolvedIndex, modifiers)`: 기본 계층 선택 진입점.
- `syncHierarchySelectionFromSelectedFolder()`: 라우팅된 폴더 인덱스에서 시각적 선택 항목을 다시 수화합니다.

<a id="modifier-recovery"></a>

## 수정자 회복

- `captureHierarchyPointerSelectionModifiers(...)`는 프레스 타임 `Cmd/Ctrl` 또는 `Shift` 상태를 저장합니다.
- `resolveHierarchySelectionModifiers(...)`는 활성화 콜백이 수정 비트 없이 도착하면 캐시된 수정자 스냅샷으로 대체됩니다.
- 이렇게 하면 `Cmd/Ctrl` 토글 선택과 `Shift` 범위 선택이 LVRS/platform 콜백 타이밍 차이 전반에 걸쳐 안정적으로 유지됩니다.

<a id="host-dependency-direction"></a>

## 호스트 종속성 방향

- 컨트롤러는 `SidebarHierarchyView` 헬퍼(예: `normalizedInteger(...)`, `invalidateHierarchySelectionVisuals()`, `resolveVisibleHierarchyItem(...)`, `hierarchyController.setHierarchySelectedIndex(...)`)에 의존합니다.
- 호스트 뷰는 래퍼 함수를 유지하여 외부 호출자는 안정적인 인터페이스를 유지하고, 선택 상태 머신은 전용 형제 파일에 저장됩니다.
