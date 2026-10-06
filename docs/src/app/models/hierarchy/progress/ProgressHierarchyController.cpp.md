# `src/app/models/hierarchy/progress/ProgressHierarchyController.cpp`

<a id="responsibility"></a>

## 책임

이 구현은 고정된 10행 LVRS 분류법에 진행 사이드바를 유지하는 동시에 `.wsnhead` 메타데이터에서 일치하는 메모 목록을 구체화합니다.

<a id="runtime-data-flow"></a>

## 런타임 데이터 흐름

- `loadFromWshub(...)`는 활성 `Progress.wsprogress`를 해석하고, 그 페이로드를 보존한 뒤, 허브 라이브러리를 `LibraryAll`를 통해 인덱싱합니다.
- `applyRuntimeSnapshot(...)`는 시작 스냅샷에서 동일한 페이로드를 새로 고침하고, 스냅샷 파일 경로에서 해당 `.wshub`를 해석하여 메모를 재인덱싱합니다.
- `setProgressState(...)`는 고정된 10-행 사이드바 분류 체계를 재구성하는 동안 지속된 페이로드를 동기화 상태로 유지하고 즉시 노트 필터링을 다시 적용합니다.

<a id="controller-hook-contract"></a>

## 컨트롤러 후크 계약

`requestControllerHook()`는 이제 진행 파일 경로가 알려지면 실제 자체 새로 고침을 수행합니다.

- `reloadFromProgressFilePath(...)`는 `Progress.wsprogress`를 재검증하고 `setProgressState(...)`를 다시 적용합니다.
- 동일한 훅을 다시 로드하면 `refreshIndexedNotesFromProgressFilePath(...)`를 통해 라이브러리 노트를 재인덱싱하여 진행 상황과 필터링된 노트 행이 함께 업데이트됩니다.
- 재로드 오류는 로드 상태 실패와 동시에 `controllerHookRequested()`를 방출하여 훅 알림 시퀀싱을 보존합니다.

<a id="note-list-semantics"></a>

## 노트 목록 의미

- `noteListModel()`는 `LibraryNoteListModel`를 노출하여 진행 영역이 공유 노트 목록 및 편집기 선택 브리지를 구동할 수 있도록 합니다.
- 진행 상황 필터링된 노트 행은 이제 선택된 노트의 RAW /source 스냅샷을 `LibraryNoteListItem::bodyText`에 보관합니다. 목록은 여전히 미리보기/검색 메타데이터만 렌더링하지만, 편집기 선택 브리지는 이제 진행 메모 목록 행 자체에서 현재 노트 본문을 부트스트랩 할 수 있으며, 비동기 재로드가 필요하기 전에 가능합니다.
- `selectedIndex`는 사이드바 행이 현재 10항목 제품 분류 체계에 고정되어 있기 때문에 정규 진행 열거값으로 간주됩니다.
- 계층 구조에 보이는 행이 있는 경우, 부정적이거나 잘못된 선택은 필터링하기 전에 첫 번째 보이는 행으로 정규화됩니다. 현재 첫 번째 진행 버킷은 `First draft`입니다.
- `refreshNoteListForSelection()` 은 선택된 진행 값과 일치하는 `.wsnhead`   `progress` 정수 값을 가진 메모만 유지하므로, 초기 진행 상태는 이제 모든 인덱스된 메모를 표시하는 대신 첫 번째 버킷으로 필터링됩니다.
- `noteDirectoryPathForNoteId(...)`는 세부 패널 전류 노트 배선과 호환되는 진행 도메인을 유지합니다.
- 본문 저장, 본문 상태 적용, 편집기 상태 새로 고침 및 단일 노트 메타데이터 재로드 API가 노트 편집기/저장 경계와 함께 제거되었습니다.

<a id="hierarchy-count-badge"></a>

## 계층 수 배지

- 이제 `depthItems()`에는 진행 행당 `count` 값이 포함됩니다.
- 배지 값은 인덱스된 노트 메타데이터에서 `progress` 정수가 행 인덱스와 동일한 노트를 계산하여 계산됩니다.
- 런타임 메타데이터 새로 고침 경로(`refreshIndexedNotesFromWshub(...)`, `refreshIndexedNotesFromProgressFilePath(...)`)는 이제 노트 새로 고침 후 `hierarchyModelChanged()`를 발행하여 재시작 전까지 진행 상황이 오래되지 않도록 합니다.

<a id="intentional-constraints"></a>

## 의도적인 제약

- 진행 행은 사용자가 편집할 수 있는 폴더가 아니라 제품 정의 지원 버킷이므로 이름 변경, 생성 및 삭제가 비활성화된 상태로 유지됩니다.
- 첫 번째 4 행은 여전히 체브론을 노출하므로, 아직 자식 노드가 구현되지 않았음에도 불구하고 컨트롤러가 Figma 정렬된 확장 상태를 유지할 수 있습니다. `setItemExpanded(...)`는 이 투영을 동기화하기 전에 공유 행 검증/상태 전환을 `IHierarchyController`에 위임합니다.

<a id="regression-checks"></a>

## 회귀 수표

- 진행 계층 구조에 진입하려면 활성 사이드바 행과 메모 목록 필터가 첫 번째 보이는 버킷에 정렬된 상태로 유지되어야 합니다.
- 런타임 페이로드 새로 고침은 행이 여전히 존재할 때마다 정규화된 첫 번째 행 대체 경로를 유지해야 합니다.
