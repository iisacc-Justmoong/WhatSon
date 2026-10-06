# `src/app/models/panel/NoteListModelContractBridge.cpp`

<a id="responsibility"></a>

## 책임

구현은 검색 텍스트에 대한 런타임 안전한 읽기/쓰기 및 이기종 노트 목록 모델 개체에 대한 현재 인덱스 선택을 제공합니다.

<a id="behavior-summary"></a>

## 행동 요약

- 다음 중 하나에서 효과적인 노트 목록 모델을 해결합니다.
  - 명시적인 `noteListModel`, 또는
  - 바운드 `hierarchyController`의 `hierarchyNoteListModel` / `noteListModel` 계약.
- `QMetaObject` 반사를 사용하여 주입된 모델이 다음을 지원하는지 여부를 감지합니다.
  - 쓰기 가능한 `searchText` 속성 또는 `setSearchText(QString)`
  - 읽기/쓰기 가능 `currentIndex` 속성 또는 `setCurrentIndex(int)`
  - 읽기 가능한 `currentNoteId` 속성
- 행별 노트 ID 를 일반 `QAbstractItemModel` 역할 맵 ( `noteId` , 다음 `id` ) 을 통해 읽으므로 QML 다중 선택은 구체적인 리스트 모델 타입을 알지 못해도 작업을 일괄 처리할 수 있습니다.
- `currentNoteEntry()` / `readCurrentNoteEntry()`를 모델 소유의 `currentNoteEntry` 속성 를 선호하고, 이후 현재 행 스냅샷으로 되돌아간 뒤 최종적으로 레거시 `currentNoteId/currentNoteDirectoryPath` 속성 로 돌아갑니다. 이는 메모 기반 계층 구조에 리소스가 `currentResourceEntry`를 통해 사용하는 것과 동일한 명시적인 "current entry" 계약 형태를 부여합니다.
- `readAllRows()` / `readAllRowsForModel(QObject*)` 스냅샷을 행 역할 이름을 `QVariantMap` 페이로드로 이동시켜 내보내지만, 전체 `bodyText` 역할은 의도적으로 건너뜁니다. `ListBarLayout.qml`는 노트 카드/리소스 카드 요약 필드만 렌더링하므로, 본문 전용 편집기 변이가 보이는 행 서명을 방해하거나 회피 가능한 리스트 스냅샷 이탈을 유발하지 않습니다.
- 사용 가능한 경우 `currentIndexChanged()`, `currentNoteEntryChanged()` 및 `currentNoteIdChanged()`에 연결하여 인덱스 기반 선택 변경이 계층 교체나 두 번째 선택 작성자를 기다리지 않고 현재 노트 ID와 현재 노트 항목을 모두 재실현합니다.
- 계층/컨트롤러가 소유한 QObject 인스턴스를 `QQmlEngine::CppOwnership`로 되돌리도록 강제하여, QML 재바인딩이 계층 전환 중에 멤버가 소유한 C++ 노트리스트 모델을 가비지 컬렉션하려고 시도하지 않도록 합니다.
- 바인딩된 모델이 삭제되면 기능 플래그를 자동으로 지웁니다.

<a id="tests"></a>

## 테스트

유지 관리되는 C++ 회귀 제품군은 이제 다음 브리지 계약을 잠급니다.
- 즉각적인 계층 컨트롤러 -> 추가 이벤트 턴을 기다리지 않고 노트 목록 모델 리바인딩
- 명시적인 note-list-model 오버라이드는 폴더/태그 메타데이터를 포함한 내보낸 행 스냅샷을 즉시 전환해야 하며, 계층-컨트롤러 입력이 동일한 전환 창에서 변경되고 있더라도 마찬가지입니다.
- 계층 소유 노트 목록 모델에 대한 자산 지원 검색/현재 인덱스/현재 노트 계약
- 도메인 전환 중 계층 소유 노트 목록 모델에 대한 QML 소유권 안정화
- 다중 선택 인덱스 -> `ListBarLayout.qml`를 제공하는 모든 노트 목록 모델에 대한 노트 ID 확인
- 현재 항목이 `LibraryNoteListModel`에서 브리지의 공개 `currentNoteEntry` 계약으로 전파되어, 메모 목록이 리소스 계층 구조의 작동 명시적 선택 패턴을 모방할 수 있습니다.
- `readAllRows()`는 라이브러리/북마크/리소스 노트리스트 모델에 대해 역할명 키가 지정된 행 스냅샷을 보존해야 하며, 이를 통해 QML 리스트 표면이 동일한 새로 고침 깜박임을 억제하면서도 `bodyText`와 같은 보이지 않는 페이로드를 계속 생략할 수 있습니다.
- `readAllRowsForModel(QObject*)`는 계층 스와프 중에도 계속 사용할 수 있어야 하며, 이를 통해 `ListBarLayout.qml`가 브리지 재바인딩 명령을 기다리지 않고 첫 번째 들어오는 스냅샷을 읽을 수 있습니다.
