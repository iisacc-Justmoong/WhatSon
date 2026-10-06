# `src/app/models/hierarchy/resources/ResourcesListModel.cpp`

<a id="responsibility"></a>

## 책임

리소스 오른쪽 패널 목록 모델에 대한 필터링 및 선택 상태를 구현합니다.

<a id="key-behavior"></a>

## 주요 행동

- `setItems(...)`를 통해 리소스 도메인 목록 항목을 허용합니다.
- 목록 페이로드 필드(`id`, 메타데이터 목록, 이미지 소스, 검색 가능한 텍스트)를 정규화합니다.
- 정제된 수신 소스 캐시를 기존 캐시와 비교한 후 교체하므로, 동일한 리소스 목록 새로 고침이 `beginResetModel()/endResetModel()` 및 `itemsChanged()` 이탈하기 전에 중지됩니다.
- `searchText`(대소문자 구분 용어 일치)를 통한 검색 필터링을 지원합니다.
- 가능한 경우 필터 및 재설정 작업 전반에 걸쳐 `currentNoteId`의 선택을 유지합니다.
- 항목 페이로드가 교체되고 동일한 ID가 여전히 존재하는 경우 선택한 리소스 ID를 유지합니다.
- `noteBacked == false`를 명시적으로 게시합니다. 따라서 선택/ID 값은 일반 목록 대리자에게 계속 제공되지만, 메모별 브리지는 이 모델을 실제 메모 소스가 아니라 리소스 브라우저로 취급해야 합니다.
- 전용 리소스 뷰어 파이프라인을 위해 선택한 행을 `currentResourceEntry` ( `type` , `format` , `resourcePath` , `resolvedPath` , `source` , `renderMode` , `displayName` , `previewText` ) 로 미러링합니다.
- 다음에 필요한 공유 선택 신호를 내보냅니다.
  - `NoteListModelContractBridge`
  - `ListBarLayout`
  - 메모에 구애받지 않는 선택 소비자만 해당

<a id="compatibility"></a>

## 호환성

이 모델은 기존 QML 대표에게 필요한 메모 카드 역할 이름을 유지하는 동시에 격리된 리소스 도메인 구현을 유지합니다.
