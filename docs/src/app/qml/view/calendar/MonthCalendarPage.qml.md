# `src/app/qml/view/calendar/MonthCalendarPage.qml`

<a id="role"></a>

## 역할

`MonthCalendarPage.qml`는 편집 영역에 대한 월 달력 오버레이 쉘을 렌더링합니다. 표면에 표시되므로 왼쪽/오른쪽으로 스와이프하면 이전 또는 다음 달로 이동합니다.

<a id="view-contract"></a>

## 계약 보기

- 입력: `monthCalendarController`
- 출력 신호: `noteOpenRequested(string noteId)`
- 후크 신호: `viewHookRequested(string reason)`
- 후크 전달자: `requestViewHook(reason)`는 `monthCalendarController.requestMonthView(...)`에 위임합니다.

<a id="pager-model"></a>

## 호출기 모델

- 페이지는 3슬롯 프로젝션 창을 유지합니다.
    - 지난달
    - 현재 표시된 달
    - 다음 달
- 이러한 예측은 이제 `MonthCalendarController.pagerMonthModels`에서 나옵니다.
- `ListView` 페이저는 이제 숫자 인덱스로 대리자를 바인딩하고 `monthProjectionForIndex(...)`를 통해 실시간 투영을 해석합니다.
- 스와이프가 완료된 후, `commitMonthSwipeDelta(...)`는 컨트롤러에서 표시되는 정규 월을 이동한 다음 로컬 호출기를 다시 중간 슬롯으로 되돌리게 합니다.
- 데스크탑은 동일한 셸을 유지하지만 대화형 스와이프 페이징을 비활성화합니다. 헤더 컨트롤은 여전히 ​​월 변경을 유도합니다.

<a id="ui-composition"></a>

## UI 구성

- 헤더:
  - 헤더 높이는 `LV.Theme.headerMinHeight - LV.Theme.gap2`(`monthHeaderHeight`)를 통해 확인됩니다.
  - 왼쪽: 활성 월 제목(`monthLabel, displayedYear`)
  - 오른쪽: 공유 `CalendarTodayControl`
- 본체:
    - 수평 `ListView` 및 `ListView.SnapOneItem`
    - 각 페이지는 달력 본문을 채웁니다. 뷰포트
    - 각 페이지는 하나의 `MonthCalendarGridSurface.qml`를 렌더링합니다.
- 헤더/본문 타이포그래피와 주중/헤더 패딩이 이제 이름이 지정된 `LV.Theme` 텍스트, 헤더 및 갭 토큰을 통해 라우팅되며, 원시 `12/22/39/54px` 리터럴이 아닙니다.
- 투명한 표면은 이제 `LV.Theme.accentTransparent`를 사용합니다.

<a id="interactiondata-flow"></a>

## 상호작용/데이터 흐름

1. `Component.onCompleted`는 이미 미리 계산된 호출기를 다시 중앙으로 가져오고 `page-open`를 요청합니다.
2. 헤더 이전/다음은 여전히 `shiftMonth(-1|1)`를 호출합니다.
4. `monthViewChanged`는 이제 공유된 `MonthCalendarController`가 이미 업데이트된 `pagerMonthModels`를 소유하고 있기 때문에 호출기만 리퍼에 추가합니다.
5. 요일 선택은 여전히 `monthCalendarController.setSelectedDateIso(...)`로 다시 위임됩니다.
6. `YearCalendarPage.qml`에서 열면, 호스트가 이 페이지가 표시되기 전에 `displayedYear`, `displayedMonth` 및 `selectedDateIso`를 사전 로드하므로, 라우팅된 월이 요청된 월/날짜에 열립니다.
7. `MonthCalendarGridSurface.qml`의 노트칩 탭이 `noteOpenRequested(noteId)`로 위쪽으로 상승하여 호스트가 편집기에서 선택한 노트를 다시 열 수 있도록 합니다.
8. `visible` 재입력은 페이저를 다시 표시하므로, 월 오버레이를 다시 열어도 헤더가 이미 현재 월을 가리키고 있는 동안 그리드가 전월 페이지에 그대로 남을 수 없습니다.

<a id="collaborators"></a>

## 협력자
- `src/app/models/calendar/MonthCalendarController.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/MonthCalendarGridSurface.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`

<a id="tests"></a>

## 테스트

- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 스와이프 완료는 호출기를 측면 슬롯에 주차해 두지 않고 표준 표시된 월을 업데이트해야 합니다.
    - 데스크톱 월별 보기는 의도하지 않은 가로 스크롤을 허용하지 않고 너비에 맞게 유지되어야 합니다.
    - 표시되는 월 페이지 내 날짜 선택은 계속해서 `selectedDateIso`를 업데이트해야 합니다.
    - 연도 캘린더의 월/일 탭에서 페이지를 열면 이전에 표시된 월 대신 요청된 월이 즉시 표시되어야 합니다.
    - 초기 월 페이지 열기는 헤더에 이미 현재 월 제목이 표시되는 동안 이전 월 그리드를 표시하지 않아야 합니다.
    - 보이는 월간 메모 칩을 클릭하거나 탭하면 `noteOpenRequested(...)`가 페이지 밖으로 나타나야 합니다.
