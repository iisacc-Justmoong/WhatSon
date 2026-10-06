# `src/app/models/detailPanel/DetailPanelController.hpp`

<a id="responsibility"></a>

## 책임
`DetailPanelController` 는 활성 세부 정보 패널 페이지 상태, 툴바 선택 상태, 전용 `fileStat` 통계 객체, 그리고 속성 폼에 의해 사용되는 3 세부 정보 로컬 계층 구조 선택자 복사를 소유합니다. 그것은 이제 재사용 가능한 노트 세부 정보 구현 기본입니다. UI 에 마운트된 구체적인 런타임 객체는 `NoteDetailPanelController` 입니다. 이 계약은 변경 없이 이를 상속합니다.

<a id="owned-objects"></a>

## 소유된 객체
- `properties`용 `DetailPropertiesController`
- `fileStat`용 `DetailFileStatController`
- 다음에 대한 `DetailContentSectionController` 인스턴스:
  - `insert`
  - `fileHistory`
  - `layer`
  - `help`
- 다음에 대한 `DetailHierarchySelectionController` 인스턴스:
  - `projectSelectionController`
  - `bookmarkSelectionController`
  - `progressSelectionController`

<a id="public-wiring-surface"></a>

## 공용 배선 표면
- `activeContentController`
- `activeStateName`
- `noteContextLinked`
- `fileStatController`
- `toolbarItems`
- `projectSelectionController`
- `bookmarkSelectionController`
- `progressSelectionController`
- `writeProjectSelection(int index)`
- `writeBookmarkSelection(int index)`
- `writeProgressSelection(int index)`
- `assignFolderByName(const QString& folderPath)`
- `assignTagByName(const QString& tag)`
- `removeActiveFolder()`
- `removeActiveTag()`
- `setCurrentNoteListModel(QObject*)`
- `setCurrentNoteDirectorySourceController(QObject*)`
- `setTagsSourceController(QObject*)`

<a id="dependency-direction"></a>

## 의존성 방향
세부 정보 패널은 이제 사이드바 계층 구조 컨트롤러에 QML 선택자를 직접 바인딩하지 않습니다. 대신, C++ 는 해당 계층 구조 컨트롤러들을 소유된 선택자 복사 객체로 읽기 전용 옵션 소스로 주입하며, 별도의 현재 노트 컨텍스트 브릿지는 활성 노트 id 와 노트 디렉토리 경로를 해결합니다. 그 브릿지는 활성 사이드바 도메인이 노트 목록 또는 노트 디렉토리 계약을 노출하지 않을 때 마지막 유효한 노트 컨텍스트를 유지하므로, 선택자들은 `No ...` 로 떨어지는 대신 열린 노트 헤더를 계속 반영할 수 있습니다. 동일하게 로드된 헤더 스냅샷은 또한 전용 `DetailFileStatController` 에 적용되므로, 통계 탭은 속성 탭과 동일한 지속된 `.wsnhead` 캐시에서 읽습니다. `noteContextLinked` 는 패널 표면의 게이트 계약입니다: 현재 노트 id 와 디렉토리가 모두 해결되고 활성 `.wsnhead` 스냅샷이 성공적으로 로드될 때만 `true` 로 전환됩니다. `setCurrentNoteListModel(QObject*)` 또한 활성 노트 목록 모델에서 선택된 노트가 `itemsChanged()` 신호를 선택적으로 관찰하므로, 밴드외부 폴더 또는 태그 편집 후 동일한 선택된 노트는 `.wsnhead` 메타데이터 스냅샷을 강제로 새로고칠 수 있습니다. 사유 쓰기 경로는 성공적인 메타데이터 편집을 다시 적용하여 로드된 상세 헤더 스냅샷을 갱신합니다. 이제 단일 노트 리로딩 호출을 계층 도메인으로 라우팅하지 않습니다. 폴더 및 태그 추가 작업은 QML 의 관점에서 동일한 변형 형식을 공유합니다: 팝업은 표준 문자열 식별자를 `assignFolderByName(...)` 또는 `assignTagByName(...)` 에 전달하고, 컨트롤러는 새로 갱신된 헤더 스냅샷을 다시 적용하기 전에 이를 영속화합니다.

<a id="selection-semantics"></a>

## 선택 의미론
- 3 선택자 복사 객체는 인덱스 `0` 에 합성된 `No ...` 항목을 노출합니다.
- 인덱스 `0`를 `writeProjectSelection(...)`, `writeBookmarkSelection(...)` 또는 `writeProgressSelection(...)`에 전달하면 현재 `.wsnhead` 파일에서 해당 필드가 지워집니다.
- 현재 선택된 인덱스를 `write...Selection(...)` API에 전달하는 것은 무작동 성공 경로이며 메타데이터 지속성 또는 계층 구조 다시 로드 콜백을 트리거하지 않습니다.
