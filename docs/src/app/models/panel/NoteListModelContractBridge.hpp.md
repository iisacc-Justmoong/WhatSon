# `src/app/models/panel/NoteListModelContractBridge.hpp`

<a id="responsibility"></a>

## 책임

`NoteListModelContractBridge`는 동적 노트 목록 계약 조사를 `ListBarLayout.qml`에서 C++로 이동하는 QML 방향 어댑터입니다.

<a id="public-contract"></a>

## 공공 계약

- 다음 중 하나를 허용합니다.
  - 명시적인 `QObject*` `noteListModel`, 또는
  - `QObject*` `hierarchyController`의 `hierarchyNoteListModel` / `noteListModel` 계약이 현재 계층 도메인에 대한 활성 목록 모델을 식별합니다.
- 명시적인 `noteListModel` 입력은 두 입력이 모두 존재해도 여전히 승리하므로, 기존 호출자는 필요에 따라 자동 계층 구조에서 파생된 선택 경로를 재정의할 수 있습니다.
- 명시적 기능 플래그를 노출합니다.
  - `hasNoteListModel`
  - `searchContractAvailable`
  - `currentIndexContractAvailable`
- 정규화된 읽기 계약을 노출합니다.
  - `currentIndex`
  - `currentNoteEntry`
  - `currentNoteId`
  - `readCurrentNoteEntry()`는 활성 메모 기반 행 스냅샷을 단일 QVariantMap 페이로드로 내보내며, 리소스 계층 구조의 `currentResourceEntry` 패턴을 반영합니다.
  - 시각적 다중 선택 행 인덱스를 안정적인 노트 ID로 다시 전환하는 `readNoteIdAt(int)`
  - `readAllRows()`는 현재 목록 행을 역할 이름 키가 있는 스냅샷으로 내보내며, 이를 QML가 직접 리셋 `QAbstractItemModel`에 바인딩하지 않고 diff할 수 있습니다.
  - 브리지 자체의 `noteListModel` 바인딩이 반드시 정착되기 전에, 계층 전환 중에 명시적인 모델 객체에 대해 동일한 스냅샷 계약을 내보내기 위한 `readAllRowsForModel(QObject*)`
- 쓰기 도우미를 노출합니다.
  - `applySearchText(QString)`
  - `pushCurrentIndex(int)`

<a id="why-it-exists"></a>

## 그것이 존재하는 이유

`ListBarLayout.qml`는 다양한 런타임 표면을 가진 다중 계층 노트 목록 모델을 계속 지원해야 합니다. 이 브리지는 동적 속성/메서드 감지를 중앙 집중화하므로 QML 측은 반사가 많은 계약 논리보다는 렌더링 및 상호 작용 흐름에 계속 집중할 수 있습니다.

`activeNoteListModel` 또는 도구 모음/도메인 전환 중 양쪽 모두 사용. 현재 셸 와이어링은 명시적 메모 목록 모델을 전달하는 것을 선호하므로 목록 표면과 편집자 표면이 같은 턴에서 같은 객체를 소비하는 반면, 계층에서 파생된 경로는 계층 객체만 아는 호출자에게 대체 경로 로 남습니다.

또한 브리지는 이제 `currentNoteEntry` 계약을 노출하므로 노트 지원 계층은 이미 `currentResourceEntry`를 통해 리소스에 대해 작동하는 동일한 명시적 "current selection entry" 형태를 따를 수 있습니다.
