# `src/app/qml/view/calendar/MonthCalendarDayCell.qml`

<a id="role"></a>

## 역할
`MonthCalendarDayCell.qml` 는 Figma 노드 `227:9427` 에 대한 재사용 가능한 월별 일 셀 컴포넌트입니다. 그것은 하나의 불리언 인수를 통해 2 변형을 지원합니다:
- 월별 날짜 셀용 `disable: false`
- 월 그리드에 표시된 인접월 오버플로 날짜에 대한 `disable: true`

<a id="public-qml-contract"></a>

## 퍼블릭 QML 계약
- `property int dayNumber`
- `property bool disable`
- `property bool selected`
- `property bool today`
- `property int maxVisibleEntries`
- `property var entryCells`
- `signal clicked`
- `signal entryActivated(var entryCellModel)`

<a id="render-rules"></a>

## 렌더링 규칙
- 배경: `LV.Theme.panelBackground04`
- 비활성화된 변형: `opacity: 0.5`
- 선택된 상태: `LV.Theme.accent` 및 `LV.Theme.strokeThin`를 사용하여 악센트 테두리를 적용합니다.
- 오늘 상태: 셀이 선택된 날짜가 아닐 때, `LV.Theme.strokeSoft` 와 `LV.Theme.strokeThin` 를 사용하여 부드러운 테두리를 적용합니다.
- 콘텐츠 패딩: `LV.Theme.gap8`
- 요일 번호 및 오버플로 라벨 타이포그래피는 `LV.Theme.textBody`를 사용합니다.
- 데이 레이블 간격, 이벤트 행 높이 및 이벤트 행 간격은 `LV.Theme.gap10`, `LV.Theme.iconSm` 및 `LV.Theme.gap2`를 사용합니다.
- 월별 그리드 사용량에서 엔트리 칩은 `cornerRadius: LV.Theme.radiusSm`와 공유 `CalendarEventCell`에 위임됩니다.
- 오버플로 상태: 표시되는 항목 용량을 초과하면 `+N more`를 렌더링합니다.
- 부모 그리드는 이제 월 예측 내에서 이미 `dayModel.entries`에서 시작된 메모/이벤트 칩 페이로드를 통과할 수 있으므로, 눈에 보이는 캘린더 항목이 재구성된 월 스냅샷과 정렬된 상태를 유지합니다.

<a id="interaction"></a>

## 상호작용
1. 내부 `MouseArea`를 통해 전체 셀을 클릭할 수 있습니다.
2. 상위 페이지는 `clicked`를 처리하고 `MonthCalendarController`에서 선택한 날짜를 업데이트합니다.
3. 개별 노트 칩은 `entryActivated(entryCellModel)`를 방출할 수 있어, 노트 열기 제스처가 전체 데이셀 클릭 계약을 탈취할 필요가 없습니다.

<a id="collaborators"></a>

## 협력자
- `src/app/qml/view/calendar/MonthCalendarPage.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 선택한 날짜 셀은 오늘이 아닌 경우에도 강조 테두리로 렌더링되어야 합니다.
    - 오늘도 선택되면 액센트 테두리가 부드러운 오늘 테두리보다 우선해야 합니다.
    - 선택된 날짜가 아닌 경우 `dayModel.isToday === true` 에서 매핑된 일 셀은 가시적이지만 대비가 낮은 테두리로 여전히 렌더링되어야 합니다.
    - 오늘이 아닌 인접한 날짜, 선택하지 않은 오버플로 날짜는 추가된 테두리 없이 계속 렌더링되어야 합니다.
    - 셀 내부에 보이는 노트 칩은 클릭하거나 탭할 때 `entryActivated(...)`를 방출해야 합니다.
