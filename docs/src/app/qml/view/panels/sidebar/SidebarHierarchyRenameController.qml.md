# `src/app/qml/view/panels/sidebar/SidebarHierarchyRenameController.qml`

<a id="responsibility"></a>

## 책임

이 도우미는 사이드바 인라인 이름 바꾸기 트랜잭션을 소유합니다. 선택한 계층 구조 행의 이름을 바꿀 수 있는지 여부를 결정하고, 임시 레이블을 시드하고, 브리지 호출을 커밋하고, 포커스/취소를 해제합니다.

<a id="rename-target-resolution"></a>

## 대상 해상도 이름 바꾸기

- `beginRenameSelectedHierarchyItem()`는 `hostView.selectedFolderIndex`에서 선택한 계층 구조 인덱스를 확인합니다.
- 편집기를 표시하기 전에 `hostView.syncSelectedHierarchyItem(false)`와 `hostView.refreshEditingHierarchyPresentation(true)`를 호출하여 오버레이가 오래된 최상위 생성 항목이 아니라 보이는 항목 찾기를 통해 고정됩니다.
- `Qt.callLater(...)` 패스는 LVRS가 행 재생성을 완료한 후 프레젠테이션 스냅샷을 다시 새로 고칩니다.
- 생성 후 경로 는 `beginRenameHierarchyItemWhenVisible(...)` 를 사용하여 컨트롤러의 새로 선택된 인덱스를 `createFolder()` 이후 포착하고, 표시된 모델을 새로고침하며, 가능하면 안정 키로 행을 활성화하고, 행이0 기하가 아닌 상태인 경우 이후 QML 턴에서 재시도합니다. 행이 누락된 동안 인라인 이름 변경 상태는 입력되지 않으므로 입력 필드는 `All Library` 또는 다른 오래된 활성 행 위에 나타날 수 없습니다.
- 행 준비도 가드는 선택된 모델 행의 안정 키에 대해 시각적 `HierarchyItem` 식별을 확인합니다. 이렇게 하면 새로 삽입된 빈 폴더 행이 레이블을 숨기는 것을 방지하고, 입력 필드가 오래된 시스템 버킷 행 위에 여전히 위치합니다.
- 생성 후 경로는 선택 활성화를 포커스와 분리합니다. `syncSelectedHierarchyItem(...)`는 여전히 활성 LVRS 행을 정렬할 수 있지만, 인라인 이름 변경 포커스는 `scheduleHierarchyRenameFieldFocus(...)`를 통해 예약되어 `LV.InputField`에 직접 적용됩니다. 반복되는 지연 패스는 생성 후 계층 행 재생성을 흡수하며, 활성 포커스를 사이드바 루트로 다시 전송하지 않습니다.
- 새 폴더 이름 변경은 안정적인 삽입 키가 사용 가능할 때 `beginRenameHierarchyItemKeyWhenVisible(...)`를 사용합니다. 키는 호스트 뷰의 전/후 계층 구조 모델 차이에서 파생되므로, 오래된 `hierarchySelectedIndex`가 일시적으로 재설정된 경우 새로 삽입된 폴더 행이 다른 위치에 있는 동안 편집기를 시스템 버킷 위에 배치할 수 없습니다.

<a id="commit--cancel-rules"></a>

## 커밋/취소 규칙

- `commitHierarchyRename()`는 실제 이름 바꾸기를 `hierarchyInteractionBridge.renameItem(...)`에 위임합니다.
- `hostView.clearEditingHierarchyPresentation()`를 통해 캐시된 행 프레젠테이션을 커밋하고 취소한 뒤, LVRS 선택을 다시 동기화하십시오.
- 커밋과 취소는 모두 이름 변경 상태가 해제된 후에도 `hostView.syncDisplayedHierarchyModel(true)`를 강제하므로, 렌더링된 계층 행은 트랜잭션이 종료된 직후 최종 컨트롤러 상태를 반영합니다.

<a id="label-handling"></a>

## 라벨 취급

- `leafHierarchyItemLabel(...)`는 이제 계층 항목의 이스케이프된 `id/path`에서 편집 레이블을 먼저 해결하며, `/`에서 렌더링된 레이블을 순진하게 분할하지 않습니다.
- `Marketing/Sales`와 같은 리터럴 슬래시 폴더 이름은 영구 계층 경로가 `Marketing\\/Sales`인 경우 하나의 이름 변경 대상이 유지되며, 인라인 편집기에서는 더 이상 이를 `Sales` 로만 압축하지 않습니다.
- 인라인 이름을 시작하면 편집된 레이블을 숨기기 위해 `displayedHierarchyModel`가 재구성되지 않습니다. 입력 오버레이는 캡처된 행 프레젠테이션을 직접 사용하여 포커스가 적용되는 동안 새로 생성된 폴더 지오메트리를 안정적으로 유지합니다.

<a id="tests"></a>

## 테스트

- 유지 관리되는 C++ 회귀 제품군은 이제 리터럴 슬래시 폴더 이름에 대한 이스케이프 인식 이름 바꾸기 레이블 경로도 고정합니다.
- 이 도우미에 대한 회귀 체크리스트:
  - 재명명 가능한 폴더에서 `Enter`를 누르면 보이는 행 레이블을 UUID와 같은 대체 경로 값으로 교체하지 않고 인라인 입력이 열려야 합니다.
  - `Marketing/Sales`와 같은 폴더 레이블은 터미널 `Sales` 세그먼트만이 아니라 전체 리터럴 라벨을 인라인 편집기에 시드해야 합니다.
  - 이름 바꾸기를 커밋하면 오버레이가 닫힌 직후에 렌더링된 행 레이블을 복원해야 합니다.
  - 이름 바꾸기를 취소하면 마찬가지로 원래 렌더링된 행 레이블을 즉시 복원해야 합니다.
