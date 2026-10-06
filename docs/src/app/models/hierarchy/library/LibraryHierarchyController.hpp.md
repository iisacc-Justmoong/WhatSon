# `src/app/models/hierarchy/library/LibraryHierarchyController.hpp`

<a id="role"></a>

## 역할
`LibraryHierarchyController`는 라이브러리 계층 구조 상태, 노트 목록 투영 및 라이브러리 노트 변형을 소유합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 여전히 계층 기능 세트를 구현합니다.
- 이제 변형 전용 소비자가 전체 구체적인 유형을 피할 수 있도록 `ILibraryNoteMutationCapability`도 구현합니다.
- 이제 시스템 캘린더 삽입은 `ISystemCalendarStore`에 따라 달라집니다.
- 인덱스된 노트 스냅샷 접근자를 노출하여 인접한 런타임 협업자들이 현재 라이브러리 노트 메타데이터를 디스크에서 허브를 다시 파싱하지 않고도 투사할 수 있도록 합니다.
- `indexedNoteRecordById(...)`를 노출하고 `indexedNoteUpserted(...)`를 방출하여 `CalendarBoardStore`와 같은 협업자들이 전체 스냅샷 교체를 기다리지 않고 하나의 노트 변이에 반응할 수 있도록 합니다.
- 그 런타임 메모 스냅샷이 변경될 때마다 `indexedNotesSnapshotChanged()` 를 방출하므로 실제 대량 스냅샷 교체 시 런타임 협업자(예: `CalendarBoardStore`)가 동기화 상태를 유지할 수 있습니다.
- `activateNoteById(...)`를 노출하여 캘린더 오버레이와 같은 크로스서피스 호출자가 QML에서 라이브러리 버킷/검색 규칙을 재구현하지 않고도 하나의 노트에 대해 라이브러리 계층 구조를 다시 표시되거나 선택 가능한 상태로 되돌릴 수 있도록 합니다.
- 캘린더 노트 칩은 캘린더 노트 칩을 다시 열 수 있어야 하기 때문에 해당 호출 가능 항목은 공개 QML 표면의 일부로 남아 있습니다.
- 공유 확장 기능을 통해 `setItemExpanded(...)`를 노출합니다; `showChevron`가 포함된 행은 별도의 CRUD 정책이 동일한 행을 이름 바꾸기/삭제로부터 보호하더라도 확장할 수 있습니다.
- 메타데이터/디테일 패널 호출자를 위해 `noteDirectoryPathForNoteId(...)`를 노출합니다. 본문 소스 조회와 지속된 본문 상태 업데이트가 노트 편집기/세이브 경계와 함께 제거되었습니다.
- 컨트롤러 계약에서 비공개 `applyInAppLibraryScaffold()` 경로를 유지하여 `All Library`, `Drafts` 및 `Today`는 성공적으로 로드된 허브가 있든 없든, 이전이든 앱 소유 행으로 유지됩니다.
- 컨트롤러 계약에 개인 `reloadFolderHierarchyFromFoldersFile(...)` 미러 경로를 유지하여 성공적인 폴더 트리 커밋이 스테이징된 `Folders.wsfolders`에서 라이브 행을 재구성하도록 하며, 스테이징된 변이 제안을 최종 사이드바 소스로 간주하지 않습니다.
- 폴더 할당, 노트 생성/삭제, 폴더 초기화, 원노트 메타데이터 재로드와 같은 노트 배포 변형이 이제 계층 노드 표면을 다시 방출하여, 폴더 트리 구조가 변하지 않더라도 사이드바 카운트 레이블이 동기화된 상태를 유지하도록 합니다.
- `applyHierarchyMove(...)` 를 `IHierarchyReorderCapability` 에서 명시적인 타겟 이동 도우미로 노출합니다. 사이드바의 일반적 LVRS 드래그/드롭 커밋은 최종 트리 스냅샷과 함께 `applyHierarchyNodes(...)` 를 통해 여전히 진행됩니다.
