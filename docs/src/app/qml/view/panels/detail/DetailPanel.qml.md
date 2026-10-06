# `src/app/qml/view/panels/detail/DetailPanel.qml`

<a id="responsibility"></a>

## 책임
`DetailPanel.qml` 는 이제 오른쪽 세부 사항 열을 위한 라우트 인식 라우터입니다. 이제 직접 노트 세부 사항 폼을 렌더링하지 않습니다. 대신 캘린더 콘텐츠가 활성화되었는지, 활성 계층 구조가 전용 리소스 계층 구조인지 여부를 결정하고, 3 개의 구체적인 서페이스 중 하나를 마운트합니다:

- `CalendarDetailPanel.qml`
- `NoteDetailPanel.qml`
- `ResourceDetailPanel.qml`

<a id="key-contracts"></a>

## 주요 계약
- `noteDetailPanelController`
- `resourceDetailPanelController`
- `sidebarHierarchyController`
- `resourcesHierarchyController`
- `calendarDetailActive`

<a id="behavior"></a>

## 행동
- 달력 페이지가 콘텐츠 보기에 탑재되면 라우터에 `CalendarDetailPanel.qml`가 표시됩니다.
- 활성 계층 제어기가 `resourcesHierarchyController`와 일치하면 라우터는 메모 상세 표면을 숨기고 `ResourceDetailPanel.qml`를 표시합니다.
- 다른 모든 계층은 계속해서 `NoteDetailPanel.qml`를 마운트합니다.
- 캘린더 상세 상태는 현재 콘텐츠 슬롯이 캘린더에 소유되어 있기 때문에 리소스 계층 상태보다 우선합니다.
- 외부 `DetailPanel` 쉘은 `RightPanel.qml`와 페이지 레이아웃에 대해 안정적으로 유지되지만, 마운트된 뷰와 실제 상세 컨트롤러가 이제 계층 도메인에 따라 분기됩니다.
