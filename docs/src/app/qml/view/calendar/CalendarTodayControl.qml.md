# `src/app/qml/view/calendar/CalendarTodayControl.qml`

<a id="role"></a>

## 역할
`CalendarTodayControl.qml`는 모든 달력 페이지(`Day`, `Week`, `Month`, `Year`)에서 사용되는 공유 `Prev / Today / Next` 탐색 컨트롤을 제공합니다.

<a id="figma-alignment"></a>

## Figma 정렬
- 참조 노드: `238:7843`
- 레이아웃 계약:
  - 수평 스택 간격: `LV.Theme.gap2`
- 모든 3 버튼은 버튼 범위에 `LV.Theme.gap20` 를 사용하고 아이콘 범위에 `LV.Theme.iconSm` 를 사용합니다.
  - 버튼 패딩은 `LV.Theme.gap2`를 통해 두 축 모두에서 `2`를 유지합니다.
  - 버튼 배경은 유휴/호버/눌림/비활성화 상태에서 `LV.Theme.panelBackground12`입니다.
  - 이전/다음 사용 `generalchevronUpLarge` 회전 `-90` / `90`
  - 중앙 작업은 LVRS `threadAtBreakpoint` 아이콘을 사용하여 Figma 주황색 원 문자 모양과 일치합니다.
  - 버튼 모서리 반경은 `LV.Theme.radiusSm`로 유지됩니다.
  - 버튼은 명확한 배경색과 함께 `LV.AbstractButton.Borderless`를 사용합니다.

<a id="public-qml-contract"></a>

## 퍼블릭 QML 계약
- `signal previousRequested`
- `signal todayRequested`
- `signal nextRequested`

<a id="lvrs-token-notes"></a>

## LVRS 토큰 노트
- 버튼 및 아이콘 범위는 원시 크기 조정 픽셀 리터럴 대신 `LV.Theme.gap20` 및 `LV.Theme.iconSm`를 사용합니다.
- `signal viewHookRequested(string reason)`

<a id="interaction"></a>

## 상호작용
1. 이전 아이콘은 `previousRequested` 및 `viewHookRequested("previous")`를 방출합니다.
2. 가운데 아이콘 버튼은 `todayRequested` 및 `viewHookRequested("today")`를 방출합니다.
3. 다음 아이콘은 `nextRequested` 및 `viewHookRequested("next")`를 방출합니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 공유 캘린더 컨트롤은 텍스트 `Today` 레이블 없이 3 LVRS 스케일의 아이콘 버튼으로 렌더링되어야 합니다.
    - 가운데 버튼은 주황색 `threadAtBreakpoint` 문자 모양을 사용해야 합니다.
    - 이전 및 다음 버튼은 Figma 노드에서 회전된 갈매기 모양 처리를 유지해야 합니다.

<a id="collaborators"></a>

## 협력자
- `src/app/qml/view/calendar/DayCalendarPage.qml`
- `src/app/qml/view/calendar/WeekCalendarPage.qml`
- `src/app/qml/view/calendar/MonthCalendarPage.qml`
- `src/app/qml/view/calendar/YearCalendarPage.qml`
