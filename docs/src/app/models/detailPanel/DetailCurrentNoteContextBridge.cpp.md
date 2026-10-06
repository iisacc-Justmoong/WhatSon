# `src/app/models/detailPanel/DetailCurrentNoteContextBridge.cpp`

<a id="responsibility"></a>

## 책임

세부 정보 패널의 현재 노트 ID 및 노트 디렉터리 컨텍스트를 활성 오른쪽 패널 목록 모델 및 활성 계층 보기 모델과 동기화된 상태로 유지합니다.

<a id="key-behavior"></a>

## 주요 행동

- 활성 목록 모델이 실제로 노트 지원되는 경우 먼저 `currentNoteEntry`에서 활성 노트 컨텍스트를 읽습니다.
- 레거시 `currentNoteId/currentNoteDirectoryPath` 속성로 전환하는 것은 엔트리 계약이 없거나 불완전할 때만 적용되며, 따라서 오래된 노트 리스트 모델은 읽을 수 있는 상태를 유지하고, 노트 기반 계층은 리소스에서 이미 사용하고 있는 동일한 명시적 현재 진입 패턴에 수렴합니다.
- 노트 기반 리스트 모델이 읽을 수 있는 현재 선택 계약을 노출하지만 해당 계약이 명시적으로 비어 있는 경우, 브리지는 이제 이전 노트 컨텍스트를 유지하는 대신 `currentNoteId/currentNoteDirectoryPath`를 초기화합니다.
- 오래된 컨텍스트 보존은 이제 활성 모델이 읽을 수 있는 메모 선택 식별 계약을 전혀 드러내지 않는 보다 좁은 경우에만 예약됩니다.
- `currentIndexChanged()`, `currentNoteEntryChanged()`, `currentNoteIdChanged()`, 및 `currentNoteDirectoryPathChanged()` 를 바운드 노트 목록 모델에서 감청하여, 현재 항목이 단일 속성 작성자가 항상 실행될 것이라고 가정하는 대신 인덱스 변경 사항에서 다시 재현되는 리소스 세부 정보 패널 패턴을 반영합니다.
- 활성 리스트 모델이 `noteBacked == false`를 노출하면, 브리지는 이제 해당 모델을 비노트 브라우저로 간주하고 명시적인 빈 노트 컨텍스트를 해결합니다.
- 이는 리소스 도메인 리스트 모델이 공유된 `currentNoteId` 속성 이름을 일반 리스트 선택을 위해 재사용한다는 이유만으로 실제 메모 패키지로 해석되는 것을 방지합니다.
- 디렉터리 해석은 현재 항목이 아직 `noteDirectoryPath`를 제공하지 않을 때만 `noteDirectoryPathForNoteId(QString)`를 통해 라우팅됩니다.

<a id="regression-checks"></a>

## 회귀 수표

- Resources 계층으로 전환하려면 `.wsresource` 패키지 디렉터리에서 `.wsnhead` 메타데이터를 로드하려고 시도하는 대신 detail-panel note-header 컨텍스트를 명확히 해야 합니다.
- 라이브러리로 다시 전환하려면 다음 새로 고침 차례에 라이브러리 노트 목록 모델에서 노트 컨텍스트를 다시 작성해야 합니다.
- `currentNoteEntry.noteDirectoryPath`가 채워진 라이브러리 노트는 소스 컨트롤러의 레거시 리졸버가 동일한 노트 ID에 대해 다른 위치를 반환하더라도 해당 경로를 유지해야 합니다.
- 읽을 수 있지만 비어 있는 `currentNoteEntry`를 노출하는 노트 기반 모델은 이전 노트 ID/디렉터리 경로를 마운트한 상태로 두지 말고 상세 패널 노트 컨텍스트를 삭제해야 합니다.
