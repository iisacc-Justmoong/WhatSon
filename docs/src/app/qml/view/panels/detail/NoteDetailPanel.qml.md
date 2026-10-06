# `src/app/qml/view/panels/detail/NoteDetailPanel.qml`

<a id="responsibility"></a>

## 책임
`NoteDetailPanel.qml`는 `DetailPanel.qml` 내부에 직접 거주하던 콘크리트 노트 디테일 표면입니다. 노트 세부 정보 도구 모음, 활성 상태/컨텐츠 해상도 및 공유 `DetailContents.qml` 양식 마운트를 소유합니다.

<a id="view-model-contract"></a>

## 뷰 모델 계약
- `property var noteDetailPanelController`
- `activeContentController`
- `fileStatController`
- `projectSelectionController`
- `bookmarkSelectionController`
- `progressSelectionController`
- `activeStateName`
- `noteContextLinked`
- `toolbarItems`

<a id="behavior"></a>

## 행동
- 이 구성 요소는 이전 모노리식 디테일 패널의 중앙 툴바와 LVRS 스케일링된 콘텐츠 크기 규칙을 유지합니다.
- 툴바 클릭은 주입된 노트 디테일 컨트롤러의 `requestStateChange(stateValue)`로 계속 이동합니다.
- `linked`에는 도구 모음과 `DetailContents.qml`가 표시됩니다.
- `detached`는 노트 세부 정보 표면을 비워 유지하므로 노트 컨텍스트가 바인딩되지 않은 경우 오래된 노트 메타데이터가 렌더링될 수 없습니다.
- 기본 패널 너비 및 패널 간격은 크기 조정된 픽셀 리터럴 대신 `LV.Theme` 토큰이라는 이름을 사용합니다.
