# `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`

<a id="lvrs-token-notes"></a>

## LVRS 토큰 노트
- 컴팩트 바닥글 버튼 배경과 바닥글 간격은 `LV.Theme.accentTransparent` 및 `LV.Theme.gapNone`를 사용합니다.
- LVRS는 현재 명명된 스크롤 물리 속도/감속 토큰을 노출하지 않으므로, 계층 구조의 동적 스크롤링은 기존의 스케일된 물리 상수를 유지하고 시각 색상, 간격 및 고정 범위는 명명된 `LV.Theme` 토큰을 사용합니다.

<a id="role"></a>

## 역할
이는 계층 구조 사이드바의 기본 시각적 호스트입니다.

계층 구조 트리, 검색/머리글/바닥글 어포던스, 도구 모음 전환, 오버레이 배치 이름 바꾸기, 노트 드롭 호버 피드백 및 북마크별 팔레트 시각적 개체를 렌더링합니다. QML 트리에서 가장 재사용 가능한 사이드바 표면입니다.

<a id="composition-model"></a>

## 구성 모델
파일은 탑재된 뷰 내부의 인라인 도우미 `QtObject`에 여러 가지 책임을 위임합니다.
- `hierarchySelectionController`: 계층 다중 선택 상태, 수정자 복구 및 기본 활성화 라우팅.
- `renameController`: 레이블 정규화의 이름을 바꾸고 트랜잭션 처리의 이름을 바꿉니다.
- `noteDropController`: 계층 적중 테스트, 드래그 페이로드 디코딩, 노트 드롭 미리보기 및 커밋 삭제.
- `bookmarkPaletteController`: 북마크 색상 토큰 조회 및 캔버스 글리프 그리기.

이러한 도우미는 생성 시 필요한 종속성을 명시적으로 바인딩해야 합니다. 그렇지 않으면 작업공간 셸이 LVRS 경로를 인스턴스화할 수 있지만 `SidebarHierarchyView.qml`를 생성하는 동안 실패하여 시작 런타임 도메인이 성공적으로 로드된 경우에도 앱에 빈 창이 표시됩니다.

북마크 행은 의도적으로 단일 `bookmarksbookmark` 아이콘 토큰을 유지하지만 북마크 도메인은 C++에서 색상별 `iconSource` 재정의를 제공할 수도 있습니다. 따라서 QML 팔레트 컨트롤러는 북마크 아이콘 색상 식별을 위한 기준 원본가 아닌 시각적 도우미입니다.

루트 파일은 호출자가 안정적인 인터페이스를 사용할 수 있도록 이러한 도우미에 대한 래퍼 함수를 계속 노출합니다. 이제 동일한 래퍼 규칙이 계층 구조 선택에 적용되므로 포인터 선택 동작이 대리인 호출 사이트를 변경하지 않고 루트 파일 밖으로 이동되었습니다.

<a id="important-inputs"></a>

## 중요한 입력
- `hierarchyController`: 활성 계층 상태 제공자.
- `hierarchyInteractionBridge`: 이름 바꾸기, 생성, 삭제, 단일 행 확장 및 대량 확장 브리지.
- `hierarchyDragDropBridge`: 재정렬 및 메모 드롭 브리지.
- 패널 색상, 삽입, 도구 모음 아이콘 이름 및 검색 구성과 같은 모양 컨트롤.

<a id="surface-contract"></a>

## 표면 계약
- `panelColor`는 데스크톱에서 투명하므로 사이드바는 공유 `ApplicationWindow` 캔버스를 상속합니다.
- `searchFieldBackgroundColor`는 데스크톱에서도 투명하며, 두 번째 검색 패널 슬래브 없이 인라인 텍스트 어포던스만 남아 있습니다.
- 이 뷰 아래에 사용되는 LVRS 행 프리미티브는 비활성 계층 행을 투명하게 유지하므로, 활성, 호버, 눌림 및 드래그 상태만 명시적 채우기를 페인트합니다.
- 보이는 계층 모델은 일반적으로 활성 컨트롤러의 공유된 `WhatSonHierarchyModel` ( `hierarchyController.hierarchyItemModel` )에 직접 바인딩됩니다. Resources는 유일한 사이드바 예외이며, 이는 컨트롤러가 게시한 `hierarchyNodes` 스냅샷을 렌더링하여 LVRS 체브론 클릭이 `QAbstractItemModel` 무효화 경로를 강제하는 대신 클릭된 행에 로컬에 머무르게 합니다.
- LVRS 행 드래그 커밋은 `HierarchyDragDropBridge.applyHierarchyReorder(...)`와 `hierarchyReorderCommitModel()`를 사용합니다. 해당 헬퍼는 LVRS가 편집 가능한 이동을 적용한 후, `items()` 메서드를 통해 공유된 `WhatSonHierarchyModel`를 스냅샷하고, 그 후 브리지는 최종 깊이 배열 결과를 지속합니다.
- 이제 내장된 `LV.Hierarchy`를 통해 공유 `TapHandler`가 플릭 테이크오버를 명시적으로 승인할 수 있습니다.
- 이제 호스트는 `LV.Hierarchy.listOvershootEnabled`, `listFlickDeceleration`,

<a id="important-outputs"></a>

## 중요한 산출물
- `searchSubmitted(...)`
- `searchTextEdited(...)`
- `hierarchyItemActivated(...)`
- `toolbarIndexChangeRequested(...)`
- `viewHookRequested`

이러한 신호는 파일을 하드 코딩된 일회성 사이드바 대신 재사용 가능한 시각적 표면으로 만듭니다.

<a id="entry-and-event-reload-hooks"></a>

## 항목 및 이벤트 다시 로드 후크

- `hierarchyController`가 변경(도메인 입력/스위치)될 때, 활성 도메인이 이를 제공하면 뷰가 `hierarchyController.requestControllerHook()`를 호출합니다.
- `onHierarchyNodesChanged`는 더 이상 `requestControllerHook()`를 직접 호출하지 않습니다. 대신, LVRS 선택/초점 프레젠테이션만 재동기화합니다.
- `onHierarchyNodesChanged`는 공유 모델 계층 구조에 대한 뷰 소유 렌더링 모델을 더 이상 재구성하지 않습니다. C++ 정책 헬퍼의 확장 상태를 캡처하고, 일시적인 메모 드롭 상태를 해제하며, 실시간 공유 항목 모델에 대해 선택/초점 표시를 재동기화합니다. Resources는 직접 `hierarchyNodes` 스냅샷 렌더 바인딩을 유지하며, 사전 공유 모델 체브론 동작과 일치합니다.
- 이제 `requestHierarchyControllerReload(reason)`는 프로젝션 새로 고침 중에 도메인 후크가 `hierarchyModelChanged`를 내보낼 때 재귀적 다시 로드 루프를 방지하기 위해 `reason == "hierarchy.nodes.changed"`를 명시적으로 무시합니다.

<a id="selected-row-activation-contract"></a>

## 선택된 행 활성화 계약

- `syncSelectedHierarchyItem(...)`는 이제 `selectedHierarchyItemActivationKey()`를 통해 안정적인 선택된 행 키를 해석합니다.
- 프로그래매틱 활성화는 이제 `LV.Hierarchy.activateListItemByKey(...)`를 선호하며, 사용 가능한 키가 없을 때만 `activateListItemById(...)`로 되돌아갑니다.
- 이는 새 계층 항목이 생성되고 주변 행이 LVRS 모델 새로 고침 타이밍에 의해 재인덱싱될 때, 삽입 후 인덱스 드리프트가 잘못된 행을 활성화하는 것을 방지합니다.
- 폴더 생성은 이제 컨트롤러에서 기본 계층 선택을 즉시 다시 동기화하고, 생성된 행을 활성 LVRS 항목으로 승격하며, 활성 도메인이 이름 변경을 지원하면 인라인 이름 변경을 시작합니다. 이렇게 하면 새로 생성된 계층 항목이 부모 행에 포커스가 고정된 대신 직접 사용자 편집을 위해 준비된 상태를 유지합니다.
- 인라인 이름 변경이 활성화된 동안 선택 재동기화는 사이드바 루트로 초점을 다시 강제로 이동시키지 않아야 합니다. 이름 변경 컨트롤러는 새로 생성된 폴더를 즉시 입력할 수 있도록 몇 개의 지연된 QML 턴 동안 LVRS 입력 필드에 초점을 다시 적용합니다.
- 생성 후 인라인 이름 변경은 모델 차이에서 삽입된 행을 해결해야 하며, 일시적 선택 인덱스에서 해결해서는 안 됩니다. 이 뷰는 `createFolder()` 이전에 계층 구조 모델을 포착하고, 변형 후 라이브 공유 모델을 읽으며, 새로 삽입된 안정 행 키를 찾은 후, 이름 변경 컨트롤러가 해당 키로 재시도할 수 있도록 합니다. 이는 실제 빈 폴더 행이 트리의 더 낮은 곳에 삽입되는 동안 `All Library` 와 같은 보호 버킷에 입력 오버레이가 고정되는 것을 방지합니다.
- 컨트롤러가 `selectedFolderIndex < 0`를 보고하면, 사이드바는 인라인 편집 프레젠테이션 외에 LVRS 활성 계층 구조 항목, 활성 ID 및 활성 키를 삭제합니다. 이렇게 하면 삭제된 포커스 폴더가 마지막으로 보이는 행에 시각적으로 포커스를 전달하지 않게 됩니다.
- 삭제 경로는 `deleteSelectedFolder()` 직후 선택 없음 동기화를 즉시 적용하고, 다음 QML 턴을 위해 다른 패스를 예약합니다. LVRS 는 컨트롤러가 이미 선택을 지운 후 계층 구조 행을 재생성할 수 있기 때문입니다.

<a id="multi-selection-contract"></a>

## 다중 선택 계약

- 계층 행 활성화는 이제 기존 확장‐억제 가드 이후에 `requestHierarchySelection(item, resolvedIndex, modifiers)`를 통해 라우팅됩니다.
- 수정자 동작:
  - 일반 클릭: 단일 선택
  - `Shift` + 클릭: `hierarchySelectionAnchorIndex`에 의해 고정된 연속 범위 선택
  - `Cmd/Ctrl` + 클릭: 추가 토글 선택
- 계층 호스트는 이제 `LV.Hierarchy`에 장착된 전용 왼쪽 버튼 `TapHandler` ( `acceptedModifiers: Qt.KeyboardModifierMask` )를 통해 프레스 타임 포인터 수정자를 캡처합니다.
- `resolveHierarchySelectionModifiers(...)`는 활성화 콜백이 수정 비트 없이 도착할 때 짧은 기간의 프레스 캐시를 사용하므로, `Cmd/Ctrl`와 `Shift` 제스처는 플랫폼 콜백 타이밍 차이에 관계없이 안정적으로 유지됩니다.
- `selectedHierarchyIndices`는 시각적 다중 선택 집합을 저장하고, `hierarchyController.setHierarchySelectedIndex(...)`는 여전히 기본 라우팅 폴더를 추적합니다.
- LVRS 계층 행은 하나의 네이티브 활성 항목만 노출하므로, 추가 선택은 `hierarchySelectionOverlayLayer`가 `selectedHierarchyOverlayRects`를 통해 렌더링됩니다.
- `selectedHierarchyIndices`는 `SidebarHierarchySelectionController.selectedIndices`의 별칭이므로, 시작 바인딩 처링이 컨트롤러가 기본값인 `[]` 값을 다시 게시하기 전에 잠시 `undefined`로 노출될 수 있습니다.
- 따라서 변경 핸들러에서 직접 `.length` 검사는 `safeSelectedHierarchyIndices`를 통해 라우팅되어야 하며, 이는 별칭을 배열로 정규화하고 시작 또는 도메인 교환 시 `undefined.length` 런타임 예외를 방지합니다.
- `collectSelectedHierarchyOverlayRects()`는 항상 배열을 반환해야 합니다. 해당 도우미가 `return overlayRects;` 없이 실패하면, 오버레이 레이어는 `undefined`에서 `.length`를 읽어들이며, 사이드바는 런타임에서 다중 선택 프레젠테이션을 잃게 됩니다.

<a id="footer-view-options"></a>

## 바닥글 보기 옵션

- 이제 가장 오른쪽에 있는 `LV.ListFooter` 메뉴 버튼을 누르면 바닥글 가장자리에 고정된 전용 `LV.ContextMenu`가 열립니다.
- 바닥글 툴바 순서는 `hierarchyFooterToolbarButtons`를 통해 고정됩니다: 왼쪽에서 오른쪽으로는 `New Folder`, `Delete Selected Folder`, 그 다음 `Open Context Menu`입니다. 세 번째 슬롯은 LVRS `generalmoreHorizontal`를 사용하며, 설정 아이콘이 아니라 메뉴 공개 옵션이기 때문입니다.
- `LV.ListFooter` 버튼은 레거시 버튼 구성 `onClicked` 콜백과 `onButtonClicked` / `handleHierarchyFooterButtonClicked(...)`를 통해 라우팅됩니다. 두 경로는 QML에서 `requestHierarchyFooterAction(...)`로 해석되며, 이는 `requestCreateFolder()`, `requestDeleteFolder()` 또는 `requestViewOptions()`를 직접 호출하고 하나의 디스패치 턴을 통합하여 콜백과 신호를 모두 발생시키는 LVRS 버전이 두 번 생성/삭제/열리지 않도록 합니다.
- 이제 컴팩트 바닥글/메뉴 측정항목이 `LV.Theme.gap2`, `LV.Theme.gap4` 및 명명된 토큰 구성을 통해 라우팅됩니다.
- 기본 트리 컨텍스트 메뉴는 `hierarchyTreeContextMenuItems`입니다. `Expand All` 및 `Collapse All` 작업을 영어로 표시하며, 이는 프로젝트 지향 문자열이 영어로 유지된다는 저장소 규칙과 일치합니다.
- 이전 `hierarchyViewOptionsMenuItems` 호환성 별칭은 의도적으로 유지되지 않으며, 트리 컨텍스트 메뉴 항목과 트리거 콜백은 현재 `hierarchyTreeContextMenuItems` 객체에 저장되어 있습니다.
- 대량 확장은 현재 예상 계층 모델에 `showChevron: true`가 포함된 행이 최소 하나 이상 포함된 경우에만 활성화됩니다.
- 두 동작은 모두 `SidebarHierarchyInteractionController.requestBulkExpansion(...)` 를 통해 라우팅되어 사이드바 확장 상태를 유지한 후 `HierarchyInteractionBridge.setAllItemsExpanded(...)` 을 호출하며, 영속화된 `expanded` 상태는 뷰가 아닌 활성 도메인 컨트롤러가 소유합니다. 그들의 뷰 훅 이유는 `hierarchy.contextMenu.expandAll` 와 `hierarchy.contextMenu.collapseAll` 입니다. 확장 후 선택된 행 동기화는 QML 에서 `Qt.callLater(...)` 에 의해 예약됩니다. 이는 확장 변이 정책이 아닌 뷰 프레젠테이션 후속 작업이기 때문입니다.
- 메뉴 항목은 이제 명시적인 `eventName` 값( `hierarchy.expandAll` / `hierarchy.collapseAll`)을 포함하며, `onItemTriggered(index)`와 `onItemEventTriggered(eventName, ...)`를 모두 수용하는 공유 트리거 핸들러를 통해 라우팅되어, LVRS 컨텍스트 메뉴 디스패치에서 콜백 유형 차이가 발생하는 대량 확장 작업을 중단하는 것을 방지합니다.
- 동일한 메뉴 항목은 또한 직접 `onTriggered` 콜백을 제공합니다. `LV.ContextMenu`는 이벤트 사양, 아이템 콜백 및 일반 아이템 트리거를 한 번의 클릭 경로로 전송하므로, `hierarchyViewOptionsTriggerQueuedAction`는 이러한 회전을 통합하고 한 번의 사용자 클릭으로 정확히 하나의 대량 확장/축소 요청을 수행하도록 보장합니다.

<a id="folder-context-menu"></a>

## 폴더 상황에 맞는 메뉴

- 동일한 `LV.ContextMenu` 인스턴스가 이제 두 번째 팝업 소유자를 생성하는 대신 라이브러리 폴더 오른쪽 클릭 작업에 재사용됩니다.
- 폴더 컨텍스트 메뉴 포인터 호출은 `openHierarchyFolderContextMenuFromPointer(...)`를 통해 중앙 집중화되므로 필터링됩니다.
- 인라인 `noteDropController.noteDropTargetAtPosition(...)`를 통해 테스트 경로를 마우스 오른쪽 버튼으로 클릭하면, 포인터가 구체적인 눈에 보이는 계층 행 위에 있을 때만 메뉴가 열립니다.
- 폴더 메뉴는 의도적으로 라이브러리 폴더 노드(`folder:*` / `uuid` 백업 항목)로 제한됩니다. `All`, `Draft` 및 `Today`와 같은 보호된 시스템 버킷은 이 메뉴를 열지 않습니다.
- 메뉴가 열리기 전에, 클릭한 폴더가 기존 선택 라우팅 경로를 통해 기본 계층 선택으로 승격됩니다. 이렇게 하면 사이드바의 다른 곳에서 사용된 동일한 selected 폴더 상태와 컨텍스트 메뉴 작업이 정렬되도록 유지됩니다.
- 메뉴는 현재 2 개의 액션만 노출합니다.
  - `New Folder`는 클릭한 폴더를 선택한 후 기존 `HierarchyInteractionBridge.createFolder()` 경로로 전달되어, 라이브러리 컨트롤러가 해당 폴더의 자식 폴더로 새 폴더를 생성합니다. 생성 후, 사이드바가 새 행을 활성 선택으로 이동시키고 인라인 이름 바꾸기를 시작합니다.
  - `Delete Folder`는 클릭한 폴더를 선택한 후 기존 `HierarchyInteractionBridge.deleteSelectedFolder()` 경로로 전달됩니다. 컨트롤러는 삭제 후 다음 또는 마지막 남은 행을 선택하는 대신 계층 포커스를 초기화하고, 사이드바는 LVRS 행 새로 고침 턴 후에 해당 삭제된 포커스를 다시 적용합니다.
- 이 동작에 대해 새로운 backend/service 객체가 도입되지 않으며, 사이드바는 기존 선택 브리지, CRUD 브리지 및 메뉴 팝업만 재사용합니다.

<a id="expansion-routing-guard"></a>

## 확장 라우팅 가드

- 확장 상태는 사용자 소유 UI 상태로 취급됩니다. `SidebarHierarchyInteractionController` 은 안정적 계층 구조 키를 통해 행의 확장을 포착하고, 각 도메인 컨트롤러는 공유 `WhatSonHierarchyModel` 를 통해 현재 상태를 게시합니다. 개수만 변경되는 경우, 폴더 구조 새로고침, 그리고 메모에서 폴더로 할당되는 새로고침은 뷰 측에서 렌더링된 복사본에 의존하는 대신 컨트롤러 소유 항목 데이터에 있는 기존 확장/축소된 행을 보존해야 합니다.
- `LV.Hierarchy` Library, Projects, Bookmarks, Tags, Progress, Event, and Preset 에 대해 공유된 모델을 직접 받습니다. `WhatSonHierarchyModel` 는 편집 가능한 플래그, 역할 쓰기, 행 이동, 그리고 `items()` 스냅샷을 노출하여 LVRS 가 QML 호환성 배열 없이 가시적인 트리를 다시 정렬할 수 있습니다. Resources 의 `hierarchyNodes` 스냅샷을 의도적으로 렌더링하는 것은 범주 트리가 더 넓고 공유 모델의 `dataChanged` 를 체vron으로 트리거하면 LVRS 가 목록의 너무 많은 부분을 무효화/재구성하게 되기 때문입니다.
- 확장 키는 활성 계층 인덱스에 의해 범위가 지정되므로, 서로 다른 계층 도메인에 있는 동일한 행 ID가 툴바가 도메인을 전환할 때 서로 확장 상태를 누출하지 않습니다.
- LVRS   `HierarchyItem`  chevron hit 타겟을 소유하고 `onListItemExpanded` 를 토글한 후 `expanded` 를 방출합니다. `SidebarHierarchyView` 는 해당 신호를 뷰 콜백으로 취급하여 `SidebarHierarchyInteractionController.handleExpansionSignal(...)` 로 전달하고, 상태가 보존된 사용자 소유 상태와 다르거나 해당 안정 행 열쇠에 대해 처음 보인 확장 신호일 때 `HierarchyInteractionBridge.setItemExpanded(...)` 를 통해 커밋합니다. Resources 스냅샷 브랜치에서 동일한 신호는 일시적인 무장 클릭 상태만 초기화하고 반환하며, 토글은 LVRS 로컬 항목 상태로 유지됩니다. 성공적인 단일 행 커밋 후 뷰는 계층 구조 모델을 다시 구축하거나 전체 사이드바 후크를 요청하지 않습니다. 변경된 도메인 컨트롤러는 `WhatSonHierarchyModel::setItemExpanded(...)` 를 호출하므로 LVRS 가 행 로컬 `expanded` 역할 업데이트를 받습니다. 별도의 포인터 팔은 대체 경로 경로로만 남게 되며, LVRS 확장 콜백을 `MouseArea` 에서 제공하지 않는 플랫폼에 대해 활성화 억제용으로만 남게 됩니다.
- `SidebarHierarchyInteractionController.captureExpansionState(...)` 는 누락된 키만 채웁니다. 새로고침된 컨트롤러/모델은 chevron 클릭으로 바로 작성된 사용자 소유 확장 값이 덮어써지지 않도록 해야 합니다.
- `syncSelectedHierarchyItem(...)`는 축소된 조상 뒤에 숨겨진 선택된 행을 활성화하지 않습니다. 이렇게 하면 모델 재구성 중에 LVRS 활성 항목 정규화가 자동으로 조상을 확장하는 것을 방지합니다.
- Chevron 기반 확장은 이제 안정적인 모델 ID(`item.itemId`, 그 다음 `item.resolvedItemId`, 콜백 `itemId`)에서 해결된 계층 인덱스를 먼저 기록하고, 모델 ID를 사용할 수 없을 때만 시각적 행 인덱스(`flatIndex`, 그 다음 콜백 `index`)로 되돌아갑니다. 그런 다음 짧은 활성화 블록 타이머가 시작됩니다.
- 왼쪽 버튼 `TapHandler` 는 `LV.Hierarchy` 에 마운트되어 누름 시간 `HierarchyItem` 와 해결된 인덱스를 기억하며, chevron LVRS 위에 차단 `MouseArea` 를 배치하지 않습니다. 방출 시간 대체 경로 커밋은 해당 행에만 `requestHierarchyChevronExpansionForTarget(...)` 를 호출하며, 방출 위치에서 계층 구조를 다시 스캔하지 않고 턴 중 나중에 포인터 아래에 있는 다른 행이 클릭에 영향을 받도록 허용하지 않습니다.
- LVRS가 이미 `onListItemExpanded`를 발행하고 커밋한 경우, 무장 키가 해제되었으며 대체 경로는 건너뛰어집니다. 그렇지 않으면 `SidebarHierarchyInteractionController.requestChevronExpansionForItem(...)`는 단일 행 접기/펼침을 수행하고, 커밋된 상태를 라이브 `LV.HierarchyItem.expanded` 속성에 다시 기록하여 가시 행이 즉시 하위 행을 숨기거나 드러내도록 합니다.
- 대체 경로 히트 테스트는 먼저 행 루트 `children` 와 LVRS `contentItem.children` 를 통해 LVRS 하위 요소를 `objectName: "hierarchyItemChevron"` 로 해결한 후 해당 슬롯의 매핑된 직사각형을 테스트하고, 더 오래된 오른쪽 가장자리 기하학 추정치로 되돌아갑니다. 이는 앱 측 포인터 팔을 실제 `HierarchyItem` chevron 슬롯과 정렬되도록 유지하며, 대략적인 행 너비 계산 대신입니다.
- 계층 행 로케이터는 일반 `children` 와 `contentItem.children` 를 모두 탐색하며, LVRS 가 내부 `Flickable.contentItem` 하위에 생성된 계층 행을 배치하기 때문에, 체vron 히트 테스트, 노트 드롭 히트 테스트, 및 해결된 행 조회는 외부 `LV.Hierarchy` 항목의 직접적인 시각적 하위 항목이 아닌 행을 반드시 확인해야 합니다.
- 사이드바는 chevron 탭을 잡기 위해 `MouseArea`  전체 바디를 `LV.Hierarchy`  위에 배치해서는 안 됩니다. 읽기 전용  도메인(예: Resources) 은 여전히 LVRS  chevron `MouseArea` 를 먼저 클릭해야 합니다. 이는 C++  확장 정책이 해당 행의 로컬 상태를 유지하기 전에 해당 행을 즉시 전환하기 때문입니다.
- `onListItemActivated`는 (`Qt.callLater`) 한 번 연기된 후, 폴더를 선택하거나 `hierarchyItemActivated(...)`를 방출하기 전에 `SidebarHierarchyInteractionController.shouldSuppressActivation()`를 통해 재검사됩니다.
- 확장 억제는 이제 해당 짧은 창에 대해 절대적으로 작동합니다: 확장 블록 타이머가 활성화된 동안에는 모든 활성화가 무시되며, 현재 선택된 행으로 해결되는 콜백도 포함됩니다.
- 이는 LVRS 내부 액티브 행 정규화가 체브론의 확장/축소가 낮은 가시 행에서 스트레이 활성화를 트리거할 때 애플리케이션 선택 상태로 새어나오는 것을 방지합니다.
- 이는 LVRS가 활성화 및 확장 콜백을 내보내는 경우에도 억제를 안정적으로 유지합니다.
- `armHierarchyExpansionActivationSuppression(...)`는 이제 `hierarchyActivationPendingSerial`를 증가시키므로, 동일한 포인터 트랜잭션에서 이미 대기 중인 활성화 콜백은 확장이 감지되면 즉시 무효화됩니다.
- 활성화 및 확장 브리지 호출은 모두 동일한 해상도의 계층 인덱스를 사용하므로, `Resources > Other`와 같은 딥 행은 LVRS가 소스 모델 ID와 다른 시각적 인덱스 값을 출력하더라도 올바른 컨트롤러 항목에 매핑됩니다.
- 정수 파싱이 이제 이 서피스에서 `Number(value) || -1`를 피하므로, 유효한 0기반 ID/색인(특히 첫 번째 행 및 진행 `0`)이 더 이상 `-1` 로 축소되지 않습니다.
- 성공적인 확장/축소 브리지 호출이 완료된 후, 뷰는 다음 턴에서 `selectedFolderIndex`를 다시 `LV.Hierarchy` 활성 프레젠테이션으로 재동기화하여 LVRS의 시각적 활성 상태가 내부 정규화된 행에 주차된 상태를 유지할 수 없습니다.
- 기본 LVRS `HierarchyItem`는 전용 체브론 상호작용 슬롯(`chevronInteractionWidth`)을 예약하고, 체브론 상호작용 플래그가 활성화된 동안 행 활성화를 차단합니다. "open the note-list page for this folder" . 체브론 탭은 계층 구조를 접거나 펼칠 수만 있어야 합니다.

<a id="drag-and-rename-behavior"></a>

## 드래그 및 이름 바꾸기 동작
- 이름 바꾸기 상태는 `editingHierarchyIndex` 및 `editingHierarchyLabel`로 표시됩니다.
- 인라인 이름 바꾸기 기하학은 더 이상 라이브 LVRS 행 객체에서만 파생되지 않습니다. 이 뷰는 이제 `editingHierarchyPresentation` 스냅샷을 유지하므로, 이름 변경 트랜잭션 중에 `LV.Hierarchy`가 항목을 재생성하더라도 오버레이가 선택된 행에 부착된 상태를 유지합니다.
- 시작 인라인 이름 변경은 렌더링된 계층 모델을 재구성하지 않습니다. 오버레이는 캡처된 행 표시를 직접 사용하고, 커밋 및 취소는 트랜잭션 상태가 해제된 후에만 확장 상태 추적 및 행 표시를 재동기화합니다.
- 새로 생성된 폴더는 선택한 폴더 행이 실시간 LVRS 계층 구조에 존재하고0가 아닌 지오메트리 스냅샷을 가지고 있을 때까지 인라인 이름 변경을 열지 않습니다. 이렇게 하면 입력 오버레이가 `All Library`와 같은 이전 활성 행을 차용하는 것을 방지하며, LVRS가 여전히 생성된 행을 재구성하고 있습니다.
- 이름 변경 행 조회는 LVRS의 시각적 행 인덱스와 안정적인 항목 키를 `standardHierarchyModel`에 대해 검증합니다. 동일한 숫자 ID를 가진 오래된 활성 항목은 입력 필드를 배치하기에 충분하지 않으며, 오버레이는 편집 중인 실제 생성된 폴더 행에 앵커를 지정해야 합니다.
- `resolveVisibleHierarchyItem(...)`는 일시적인 LVRS 활성 행 참조를 신뢰하는 대신 공유 계층‐항목 로케이터와 해당 식별 확인을 통해 이름 변경 배치를 라우팅합니다. 이렇게 하면 재명명 위치가 선택된 폴더에 연결된 상태를 유지하고, 재구성된 트리에서 가장 먼저 생성되는 행은 그대로 유지되지 않습니다.
- 노트 드롭 미리보기 상태는 `noteDropHoverIndex`로 표시됩니다.
- 파일 하단에 있는 `DropArea`는 이제 포인터 페이로드를 `noteIdsFromDragPayload(...)` 로 라우팅하므로, 다중 선택된 노트 목록 그룹에서 발생한 드래그가 선택된 모든 노트를 한 번에 해당 호버된 폴더에 할당할 수 있습니다.
- 노트 드롭 `DropArea`는 노트 리스트 드래그 키(`whatson.library.note`)에 적용됩니다. 키가 없는 계층 항목 드래그를 허용해서는 안 됩니다. 이는 해당 드래그가 LVRS `Hierarchy`로 전달되어야 트리 재배열이 `listItemMoved`를 발생시킬 수 있기 때문입니다.
- `LV.Hierarchy.onListItemMoved`는 `hierarchyReorderCommitModel()`를 `HierarchyDragDropBridge.applyHierarchyReorder(...)`를 통해 전달합니다. 헬퍼는 LVRS가 편집 가능한 행 이동을 수행한 후 공유 항목 모델을 스냅샷하고, 사이드바는 계층 구조 컨트롤러가 백킹 트리 저장소를 지속하고 재파싱하도록 허용합니다.
- 구체적인 폴더 계층 행에 메모 목록 항목을 드롭하는 것은 멤버십 변이이며, 뷰는 호버된 행 인덱스를 해석하고, 끌어온 노트 ID 배열을 `HierarchyDragDropBridge.assignNotesToFolder(...)` 를 통해 전달하며, 성공적인 커밋을 `hierarchy.noteDrop`로 처리합니다.
- `DropArea`는 `HierarchyDragDropBridge`가 연결될 때마다 계속 활성화된 상태를 유지합니다. `noteDropContractAvailable`에 스스로 게이트를 설치해서는 안 됩니다. 이는 드래그 엔터/드롭 이벤트가 컨트롤러에 전혀 도달하는 것을 방지하기 때문입니다. 컨트롤러와 도메인 컨트롤러 기능은 구체적인 목표를 수락하거나 거부할 책임이 계속됩니다.
- 빈 또는 비노트 드래그 페이로드는 명시적으로 거부되며, 노트 드롭 미리보기가 삭제됩니다. 이는 키 필터 주변에 방어적인 대체 경로이며, 오래된 노트 호버 상태가 계층 구조 재오더가 처리된 것으로 보이게 하는 것을 방지합니다.
- 그 폴더 드롭 경로는 인라인 `noteDropController.normalizeNoteIds(...)`가 구체적인 배열을 반환하는 것에 의존합니다. 헬퍼가 `return normalized;` 없이 중단될 경우, 사이드바는 모든 노트 목록에서 폴더로의 드롭을 빈 페이로드로 거부합니다.
- 동일한 `DropArea`가 이제 드롭-커밋 불리언을 `drop.accepted`에 미러링하므로, 실패한 폴더 할당 시도는 QML 드래그/드롭 계약에 의해 눈에 띄게 거부된 상태로 유지되며 처리된 것으로 보입니다.
- 노트 드롭 호버 펄스는 투명도 세그먼트 양쪽에서 명시적인 `NumberAnimation.from` / `to` 쌍을 사용하며, qmlcache 는 숫자 토큰만으로는 허용되지 않으며 키드 애니메이션 속성을 절대 대체해서는 안 됩니다.

<a id="architectural-reading"></a>

## 건축 독서
이 파일은 계층 구조 비즈니스 규칙이 존재하는 장소가 아닌 합성 뷰로 읽혀져야 합니다. 도메인이 이름을 변경하거나, 재배열하거나, 노트를 수락할 수 있는지와 같은 구체적인 지식에 대한 변경이 필요한 경우, 답은 하드 코딩된 QML 가정을 통해 얻는 것이 아니라 브릿지와 기능 인터페이스에서 나와야 합니다.

<a id="recent-updates"></a>

## 최근 업데이트
- 파일 루트에 `pragma ComponentBehavior: Bound`를 추가하여 툴바의 `Repeater` 대리자들이 바인딩된 컴포넌트 범위를 가진 `sidebarHierarchyView` ID 멤버를 참조할 수 있습니다.
- 수정자 기반 계층 다중 선택( `Cmd/Ctrl` 토글, `Shift` 범위)을 추가했으며, 비주요 선택 행에 대해 명시적인 오버레이 하이라이트 경로를 제공합니다.
- 인라인 헬퍼 컨트롤러를 `sidebarHierarchyView`, `hierarchyTree`, `hierarchyRenameField`, `HierarchyInteractionBridge`, `HierarchyDragDropBridge` 로 바인딩하고, 북마크 오버레이 캔버스를 시작할 때 필요합니다 - 속성 초기화는 작업 공간 경로를 중단할 수 없습니다.
- 포인터업 후 LVRS 활성화 콜백이 전달될 때 수정기 선택이 되돌아가지 않도록 프레스 타임 모디파이어 캡처와 캐시된 활성화 모디파이어 해상도를 추가했습니다.
- 노트 드롭 호버 펄스에서 명시적인 `NumberAnimation.from` 키를 복원하여 Xcode /qmlcache 사전 파싱이 사이드바 애니메이션 객체를 다시 허용합니다. 기존 데스크톱 라우팅 훅을 변경하지 않고 터치 라우트. LVRS는 릴리스 후 터치 모멘텀을 유지하기 위해 단독으로 기본값을 설정합니다.

<a id="tests"></a>

## 테스트

- `test/cpp/suites/sidebar_hierarchy_rename_controller_tests.cpp` 는 LVRS 푸터 버튼 순서, 직접 설정 콜백, `onButtonClicked` 신호 경로, QML 직접 디스패치, 그리고 라이브 클릭 경로에 C++ `SidebarHierarchyInteractionController` 푸터 왕복 변환 왕복 변환의 부재를 확인하여 푸터 액션 계약을 잠가둡니다.
- `test/cpp/suites/qml_editor_surface_policy_tests.cpp` 와 `test/cpp/suites/sidebar_hierarchy_controller_tests.cpp` 는 계층 확장 계약을 잠근다: 새로 갱신된 모델은 C++ 소유의 확장 상태를 활성 계층 범위로 유지해야 하며, 가려진 선택된 행은 단순히 표시를 위해 활성화되어서는 안 되며, LVRS 체vron 변경은 C++ 정책 객체를 통해 커밋되어야 한다.
- 이 파일에 대한 수정자 선택 회귀 체크리스트:
  - `Shift + click`는 `hierarchySelectionAnchorIndex`에서 연속 계층 구조 범위를 생성합니다.
  - `Cmd/Ctrl + click`는 단일 선택으로 축소하지 않고 계층 구조 행을 전환합니다.
  - 수정자 비트를 생략한 활성화 콜백은 여전히 누름 시간 수정자 의도를 존중해야 합니다.
  - 갈매기 모양 확장/축소는 기본 활성 선택 항목을 관련되지 않은 형제 행으로 이동해서는 안 됩니다.
  - Chevron 확장/축소는 이미 선택된 폴더인 경우에도 탭된 행에 대해 폴더 활성화가 발생하지 않아야 합니다.
  - 다중 선택된 노트 목록 그룹을 폴더에 놓으면 드래그한 모든 노트 ID에 대해 폴더 할당을 시도해야 합니다.
- 사이드바 노트 드롭 컨트롤러가 정규화 된 배열 반환 경로를 생략했기 때문에 폴더 드롭 수락은 항상 빈 페이로드로 후퇴해서는 안 된다.
- 실패한 노트 드롭 커밋은 `drop.accepted == false`를 떠나야 하므로, 드래그 계약은 계속해서 LVRS / Qt에 거부를 보고합니다.
- 폴더 트리 드래그/드롭은 `applyHierarchyReorder(sidebarHierarchyView.hierarchyReorderCommitModel(), itemKey)` 에서 `LV.Hierarchy.onListItemMoved` 를 계속 사용해야 하며, 두 번째 인덱스 재생 경로를 추가하면 최종 LVRS 스냅샷이 영속화되기 전에 드래그를 인정할 수 있습니다.
- `LV.Hierarchy.model`는 모든 비리소스 계층 구조에 대해 `hierarchyController.hierarchyItemModel`에 직접 바인딩된 상태를 유지해야 합니다. 리소스는 `hierarchyUsesSnapshotRenderModel`를 통해 `hierarchyNodes`에 의도적으로 바인딩됩니다; 해당 예외를 제거하면 각 클릭이 공유 모델 무효화 경로에 들어갈 수 있기 때문에 느린 chevron 응답이 다시 발생합니다.
- 노트-드롭 호버 불투명도 애니메이션은 두 개의 `NumberAnimation` 블록에 명시적인 `from:` / `to:` 키를 유지해야 하며, 이를 위해 qmlcache 파싱이 베어리 숫자 토큰에서 실패하지 않도록 해야 합니다.
- 빌드 시 회귀 가드: 첫 번째 펄스 세그먼트는 `from: 0.78`를 유지해야 하고 두 번째 펄스 세그먼트는 `from: 1.0`를 유지해야 합니다; 둘 중 하나를 빈 숫자 라인 브레이크 qmlcache 코드 생성으로 교체합니다.
- 시작 회귀 보호: 인라인 헬퍼 객체는 명시적인 의존성 바인딩을 유지해야 하며, 선택 헬퍼는 `view`를 `sidebarHierarchyView`에 지정해야 합니다; 그렇지 않으면 작업공간 라우트 구성 중에 QML가 초기화되지 않은 필요한 속성를 보고합니다.
- 변하지 않은 노드로 인한 주기적 계층 구조 새로 고침은 눈에 띄게 깜빡이지 않아야 합니다. 동등한 `hierarchyNodesChanged` 방출이 렌더링된 모델을 새 QML 스냅샷으로 교체하는 것을 유발해서는 안 되기 때문입니다.
- 라이브러리 폴더 행을 마우스 오른쪽 버튼으로 클릭하면 `New Folder` 및 `Delete Folder`가 포함된 상황에 맞는 메뉴가 열려야 합니다.
- 해당 메뉴에서 `New Folder`를 트리거하려면 기존 폴더 생성 경로를 재사용하고, 새 폴더를 클릭한 폴더의 자식 폴더로 삽입해야 합니다.
- 해당 메뉴에서 `Delete Folder`를 트리거하면 기존 삭제 경로를 재사용하고, 이전에 선택된 행 대신 클릭한 폴더를 제거해야 합니다.
- `All`, `Draft` 또는 `Today`와 같은 보호된 라이브러리 버킷을 마우스 오른쪽 버튼으로 클릭하면 폴더 컨텍스트 메뉴가 열리지 않아야 합니다.
- Enter 기반 폴더 이름 변경은 커밋 또는 취소 후 행을 비워 두어서는 안 됩니다.
- 이름을 변경한 후, 이후의 포커스/선택/컨텍스트 메뉴 상호작용은 `itemKey`, `uuid` 또는 기타 내부 식별자를 노출하는 대신 폴더 라벨을 유지해야 합니다. 키네틱 캐리와 함께 릴리스합니다. 데스크톱의 대체 경로 스크롤 계약의 사이드바를 남깁니다.
