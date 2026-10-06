# `src/app/models/detailPanel/DetailPanelController.cpp`

<a id="responsibility"></a>

## 책임
`DetailPanelController.cpp` 는 활성 세부 정보 패널 상태, 활성 섹션 뷰 모델 포인터, 툴바 선택 목록, 전용 `fileStat` 통계 컨트롤러, 그리고 속성 폼에 의해 사용되는 3 `.wsnhead` 기반 선택자 복사 객체를 소유합니다. 그 동작은 이제 `NoteDetailPanelController` 에 의해 재사용되는 구체적인 노트 세부 정보 구현입니다. 리소스 세부 정보 라우팅은 더 이상 이 노트 전용 상태 머신을 통해 강제로 수행되지 않습니다.

<a id="active-state-surface"></a>

## 활성 상태 표면
내보낸 `activeStateName()`는 이제 수정된 페이지 ID를 따릅니다.
- `properties`
- `fileStat`
- `insert`
- `layer`
- `fileHistory`
- `help`

<a id="notes"></a>

## 메모
- 이제 전용 섹션 보기 모델 개체는 내보낸 페이지 ID와 동일한 표준 이름을 사용합니다.
- 공개 문자열 계약과 내부 멤버/접근자 이름이 일치합니다. C++와 QML 사이에는 앨리어스 레이어가 없습니다.
- `fileStat` 페이지는 더 이상 일반적인 자리표시자 섹션 객체로 해석되지 않으며, 이제 활성 헤더 스냅샷에 의해 뒷받침되는 `DetailFileStatController`를 노출합니다.
- 이제 속성 양식 선택기가 전용 `DetailHierarchySelectionController` 개체로 지원되므로 사이드바의 계층 구조를 클릭해도 세부 정보 패널 콤보 상태가 다시 작성되지 않습니다.
- `main.cpp`는 이러한 복사본에 대한 옵션 소스로만 표준 프로젝트/책갈피/진행 계층 구조 컨트롤러를 삽입합니다.
- 현재 노트 컨텍스트는 이제 라이브러리 계층 구조에 고정되는 대신 `SidebarHierarchyController` 활성 바인딩을 따르므로 세부 정보 패널은 현재 작업 공간 보기가 실제로 식별한 노트에 대해 `.wsnhead` 파일을 읽고 씁니다.
- 현재 노트 브리지는 이제 활성 사이드바 도메인이 노트 목록이나 `noteDirectoryPathForNoteId(...)` 계약을 노출하지 않을 때 마지막 유효한 노트 ID와 노트 디렉터리 경로를 유지하므로 동일한 노트가 작업 공간에 열려 있는 동안 선택기 복사본이 `No ...`로 다시 축소되지 않습니다.
- 이제 `currentNoteIdChanged` 전환은 `currentNoteDirectoryPathChanged`뿐만 아니라 `reloadCurrentHeader(...)`를 즉시 트리거합니다. 이렇게 하면 활성 노트 ID가 변경되지만 확인된 노트 디렉터리 경로 문자열이 일시적으로 변경되지 않을 때 폴더/태그/프로젝트 메타데이터가 이전 노트에 달라붙는 것을 방지할 수 있습니다.
- 현재 노트 목록 모델은 이제 선택적 `itemsChanged()` 신호에서도 관찰됩니다. 활성 노트가 선택된 상태로 유지되고 메타데이터가 다른 곳에서 변경되면 세부 정보 패널은 노트 ID 전환을 기다리는 대신 동일한 `.wsnhead` 파일을 강제로 다시 로드합니다.
- `reloadCurrentHeader(...)`는 이제 로드된 헤더 스냅샷을 `DetailPropertiesController`와 `DetailFileStatController` 모두에 적용하고, 노트 컨텍스트가 유효하지 않을 경우 두 표면을 함께 삭제합니다.
- 컨트롤러는 이제 `noteContextLinked`를 내보냅니다; `reloadCurrentHeader(...)`는 활성 노트 ID/경로가 해결되고 `.wsnhead` 스냅샷이 로드 가능한 경우에만 `true`로 설정되며, 그렇지 않으면 두 콘텐츠 컨트롤러가 모두 삭제되고 패널이 분리된 것으로 표시됩니다.
- `writeProjectSelection(...)`, `writeBookmarkSelection(...)` 및 `writeProgressSelection(...)`는 활성 노트 헤더 파일에 직접 유지된 다음 파일 지원 세션 저장소에서 선택기 복사본을 다시 동기화합니다.
- 이제 수신 선택기 인덱스가 이미 세부 로컬 선택기 복사 `selectedIndex`와 일치하면 공유 쓰기 경로가 단락되므로 `No ...`를 반복하거나 동일한 옵션을 클릭해도 동일한 `.wsnhead` 상태가 다시 지속되지 않습니다.
- 디테일 패널 메타데이터 쓰기가 성공하면, 컨트롤러는 활성 헤더 스냅샷을 디테일 패널 표면에 다시 적용합니다. 계층 노트 목록 재투영은 더 이상 단일 노트 리로드 헬퍼를 통해 라우팅되지 않습니다.
- 이제 각 선택기 모델에는 합성 지우기 항목(`No project`, `No bookmark`, `No progress`)이 앞에 추가되며, 해당 항목을 선택하면 표시되는 레이블 텍스트를 쓰는 대신 현재 `.wsnhead` 파일에서 해당 필드가 지워집니다.
- `assignFolderByName(...)`는 먼저 `Folders.wsfolders`에서 폴더 항목을 확인하거나 생성한 다음 결과 폴더 경로/uuid 바인딩을 활성 노트 헤더 파일에 유지하고 마지막으로 현재 헤더를 속성 콘텐츠 뷰 모델에 다시 적용합니다.
- `assignTagByName(...)`는 선택한 태그를 공유 세션 스토어를 통해 활성 메모 헤더 파일에 영구 삽입하고, 새로 고침된 헤더를 속성 콘텐츠 뷰 모델에 다시 적용하며, 해당 태그가 이미 노트에 존재했을 때 새로 할당된 태그 행을 다시 선택합니다.
- `removeActiveFolder()` 및 `removeActiveTag()`는 동일한 파일 지원 세션 저장소를 통해 현재 노트 헤더 파일에서 활성 메타데이터 항목을 삭제한 다음 업데이트된 헤더를 속성 콘텐츠 뷰 모델에 다시 적용합니다.
- Progress 계층 소스가 주입될 때, 선택자는 옵션 목록을 현재 노트 헤더의 `progress enums="{...}"` 순서로 교체하는 대신 해당 정규 소스 계층을 그대로 유지합니다.
- 현재 노트 헤더의 진행 열거는 정규 Progress 계층 제어기가 주입되지 않을 경우 이제 대체 경로 옵션 소스만 사용됩니다.
- Progress 쓰기는 여전히 `itemId`, `progressValue`, `progress:*`와 같은 소스 메타데이터에서 저장된 정수를 해석하므로, 이 조합은 노트 로컬 행 인덱스 대신 소유 Progress 계층의 숫자 값을 반환합니다.
- 진행 지속성은 `itemId` 또는 숫자 `progress:*` 키와 같은 계층 구조 항목 메타데이터에서 열거형 정수를 확인합니다. 더 이상 기본 `Ready/Pending/InProgress/Done` 레이블에 하드 코딩되지 않습니다.
- 진행률을 지우면 `.wsnhead` 필드가 명시적인 빈 진행률 값으로 기록되며, 이는 세부 선택기로 다시 `No progress`로 왕복됩니다.
- 이제 속성 양식은 쓰기 작업을 위해 지속된 `.wsnhead` 전체 경로를 유지하면서 폴더 행을 `header.folders()`에서 추출된 리프 이름(예: `Archive/Knowledge` -> `Knowledge`)으로 렌더링합니다.
- 폴더/태그 추가 팝업은 계층 소스 컨트롤러를 직접 변경하지 않으며, 해당 계층 노드를 옵션 소스로만 읽고 실제 노트 헤더 쓰기 소유권을 `DetailPanelController` 내에서 유지합니다.
