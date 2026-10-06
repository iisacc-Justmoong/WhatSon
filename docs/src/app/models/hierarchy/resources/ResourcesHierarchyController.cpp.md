# `src/app/models/hierarchy/resources/ResourcesHierarchyController.cpp`

<a id="responsibility"></a>

## 책임

이 구현은 리소스 사이드바를 하드코딩된 정적 트리가 아닌 `.wsresource` 패키지에서 구축된 메타데이터 기반 계층 구조로 노출합니다.

<a id="tree-contract"></a>

## 나무 계약

`setResourcePaths(...)` 는 경로 문자열만 저장하지 않습니다. 각 리소스 참조를 2레벨 계층 구조로 구체화합니다:

- `type`
- `format`

계층 구조는 레거시 상호 작용 계약을 유지합니다.

- 최상위 행은 리소스 유형입니다.
- 유형을 확장하면 형식 행이 표시됩니다.
- 포맷 행은 다중 도트 파일 이름의 전체 접미사가 아니라 터미널 파일 접미사(`.png`, `.pdf`, `.gz`)로 키가 지정되므로, `...11.25.16.png`와 같은 스크린샷 이름은 `.png` 아래에 그룹화됩니다.

입력 경로 목록이 비어 있는 경우에도 컨트롤러는 기본 형식 카탈로그와 함께 상위 유형을 계속 게시하므로 리소스 사이드바는 절대 유형 전용 목록이 아닙니다.

`syncModel()`는 `depthItems()`를 공유 `WhatSonHierarchyModel`에 게시하여 라이브러리 계층 구조의 LVRS 지향 모델 계약과 일치하는 동시에 `ResourcesHierarchyItem` 구조체의 리소스 분류 소스 데이터를 유지합니다.

`setItemExpanded(...)` 는 공유 체vron 유효성 검사/상태 반전을 `IHierarchyController` 에 위임한 후 변경된 행만 위해 `WhatSonHierarchyModel::setItemExpanded(...)` 를 호출합니다. 단일 체vron 클릭에 대해 전체 모델을 다시 구축해서는 안 됩니다. 리소스 분류는 CRUD 에 대해 읽기 전용 로 유지되지만, 가시적인 타입 행은 여전히 형식 자식들을 접거나 펼칠 수 있습니다.

<a id="right-panel-list-contract"></a>

## 오른쪽 패널 목록 계약

이제 이 컨트롤러는 공유 오른쪽 패널에 대한 전용 `ResourcesListModel` 프로젝션을 소유합니다.

- 계층 구조에 표시되는 행이 있는 경우 오른쪽 패널 목록이 다시 작성되기 전에 음수 또는 유효하지 않은 `selectedIndex`가 첫 번째 표시 행으로 정규화됩니다.
- 현재 첫 번째 행은 `Image`이므로 리소스 계층을 입력하면 이제 전체 리소스 세트 대신 이미지 필터링된 목록이 기본값으로 사용됩니다.
- 선택된 `kind="type"` 행: 해당 유형의 리소스만 예상됩니다.
- 선택된 `kind="format"` 행: 유형+형식이 일치하는 리소스만 투영됩니다.

예상되는 각 행에는 다음이 포함됩니다.

- `noteId`: 해결된 `.wsresource` 패키지 디렉터리 경로(가능한 경우 절대 경로)
- `primaryText`: 해결된 자산 표시 이름
- `bodyText`: 편집기 표면에서 사용되는 정규화된 `<resource ...>` 태그 텍스트
- `image/imageSource`: 이미지 리소스만 있으므로 목록 카드에 인라인 축소판이 표시될 수 있습니다.
- `type/format/resourcePath/resolvedPath/source/renderMode/displayName` : 렌더러 및 리스트 델리게이트가 사용하는 리소스 뷰어 메타데이터

이는 리소스 목록이 사이드바가 이미 활성으로 표시한 것과 동일한 필터를 사용하여 즉시 나타나야 하는 계층-도구 모음 전환을 포함하여 리소스 도메인의 이전 `No list data` 동작을 수정하는 브리지입니다.

<a id="resource-deletion-contract"></a>

## 자원 삭제 계약

`deleteNoteById(...)` / `deleteNotesByIds(...)`는 이제 오른쪽 패널 목록 ID를 리소스 패키지 대상으로 처리합니다.

- 각 ID는 소유한 `.wsresource` 디렉터리로 다시 확인됩니다.
- 패키지 디렉터리가 삭제되기 전에 일치하는 `Resources.wsresources` 항목이 제거됩니다.
- 디렉터리 제거에 실패하면, 계층 파일 쓰기가 이전 리소스 경로 목록으로 롤백됩니다.
- 성공적인 삭제는 `hubFilesystemMutated()`를 방출하므로 허브 동기화는 해당 편집을 지역 변이로 계속 표시합니다.

이는 `Delete` / `Backspace` 키보드 삭제를 포함하여 활성 노트 목록이 리소스 도메인에 속하는 경우 `ListBarLayout`에서 사용하는 경로입니다.

<a id="idempotent-update-guard"></a>

## 멱등성 업데이트 가드

`setResourcePaths(...)`는 멱등성을 갖습니다. 경로를 정리하고 계층 구조 행을 다시 작성한 후 두 가지 모두 변경되지 않으면 조기에 반환됩니다.

- `syncModel()` 재설정 없음
- `hierarchyModelChanged` 신호 없음
- 강제 `setSelectedIndex(-1)` 없음

이 가드는 리소스 페이로드가 안정적일 때(예: 반복적인 `rawCount=0` 다시 로드) `SidebarHierarchyView`에서 후크 기반 다시 로드 루프를 방지합니다.

<a id="expansion-preservation"></a>

## 확장 보존

재구축 중에 이전 `key -> expanded` 상태가 복원되므로 런타임 스냅샷 업데이트는 이미 열려 있는 유형/형식 행을 축소하지 않습니다.

또한 `setResourcePaths(...)`는 리소스 경로 재구축 후에도 해당 행이 여전히 존재하는 경우 `key`에 의해 현재 선택된 계층 구조 행을 유지합니다. 이렇게 하면 패키지 삭제 또는 가져오기 후에도 빈 전환 상태로 돌아가는 대신 오른쪽 패널 리소스 목록이 활성 상태로 유지됩니다.

<a id="regression-checks"></a>

## 회귀 수표

- 활성 계층을 `Resources`로 전환할 때 공유 오른쪽 패널 목록을 비워두면 안 됩니다.
- 초기 리소스 입력 상태는 사이드바 포커스를 유지하고 오른쪽 패널 필터를 첫 번째 보이는 행에 정렬해야 합니다 ( `Image` ).
- `.wsresource` 패키지를 삭제하거나 가져오는 경우, 재구성 후에도 선택된 분류키가 여전히 존재할 때 현재 프로젝션을 유지해야 합니다.
- 다중 점 이미지 자산과 심지어 독이 된 `resource.xml format=".25.16.png"` 페이로드라도 가짜 `.25.16.png` 자식 항목을 생성하는 대신 정상적인 `.png` 분류 행으로 구현되어야 합니다.

<a id="load-fallback"></a>

## 대체 경로 로드

`loadFromWshub(...)`는 먼저 `.wscontents/Resources.wsresources`를 구문 분석합니다. 해당 파일이 없거나 경로가 없으면 `.wsresource` 패키지에 대한 모든 허브 수준 리소스 루트(`.wsresources` 및 `*.wsresources`)를 검색하는 것으로 대체됩니다.

따라서 이 도메인의 정식 페이로드는 `.wsresource` 패키지 경로의 정규화된 목록으로 유지됩니다.

<a id="controller-hook-contract"></a>

## 컨트롤러 후크 계약

`requestControllerHook()`는 `m_resourcesFilePath`가 알려진 경우 파일 지원 다시 로드를 수행합니다.

- `reloadFromResourcesFilePath(...)`는 `Resources.wsresources`가 있는 경우 재분석합니다.
- 파싱된 경로 목록이 비어 있는 경우, 패키지 스캔 대체 경로를 다시 적용하여 계층 구조가 여전히 디스크상의 리소스를 반영하도록 합니다.
- 행을 재구성하기 전에 리소스 참조 기본 경로(해결된 `.wshub` 루트 포함)를 재컴퓨팅하여 훅 새로 고침 후에도 미리보기/어셋 해상도를 안정적으로 유지합니다.
- 기본 경로 변경은 리소스 목록 재구현을 트리거하여, 허브 레이아웃 루트가 변경될 때 오른쪽 패널 경로가 해결된 상태를 유지하도록 합니다.

<a id="count-role-compatibility"></a>

## 카운트 역할 호환성

`depthItems()`에는 모든 행에 숫자 `count` 필드가 포함되어 있으며 `ResourcesHierarchyItem`에서 구체화된 계층 구조 수를 전달합니다.

- `type` 행은 해당 유형으로 분류된 리소스 수를 노출합니다.
- `format` 행은 해당 형식으로 분류된 리소스 수를 노출합니다.

이 값은 `LV.Hierarchy`의 오른쪽 카운트 렌더러에서 직접 사용됩니다.

<a id="regression-coverage"></a>

## 회귀 적용 범위

- `resourcesHierarchyController_publishesDepthItemsToSharedModel`는 `depthItems()`와 공유된 `WhatSonHierarchyModel::items()` 스냅샷이 일치하도록 확인합니다.
- `resourcesHierarchyController_updatesChevronExpansionThroughSharedModelRow`는 리소스 체브론 변경이 `modelReset` 없이 행 로컬 `dataChanged`를 방출하는지 확인합니다.
