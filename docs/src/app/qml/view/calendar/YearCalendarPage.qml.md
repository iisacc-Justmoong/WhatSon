# `src/app/qml/view/calendar/YearCalendarPage.qml`

<a id="role"></a>

## 역할
`YearCalendarPage.qml`는 연간 달력 콘텐츠 화면을 렌더링하고 이제 보드 스타일의 일일 항목 수를 표시하므로 연도 보기가 상위 수준 이벤트/작업 히트맵 역할을 합니다.

<a id="view-contract"></a>

## 계약 보기
- 입력: `yearCalendarController`
- 후크 신호: `viewHookRequested(string reason)`
- 항법 신호: `monthCalendarOpenRequested(int year, int month, string selectedDateIso)`
- 후크 전달자: `requestViewHook(reason)`는 `yearCalendarController.requestYearView(reason)`에 위임합니다.

<a id="ui-composition"></a>

## UI 구성
- 표면:
  - 루트 페이지는 `LV.Theme.accentTransparent`를 유지하므로 앱 배경이 표시됩니다.
- 헤더:
  - 공유 `CalendarTodayControl`(`Prev/Today/Next`)는 항상 표시되며 연도 이동/초점 작업을 유도합니다.
  - 달력 시스템 전환은 더 이상 이 페이지에 표시되지 않습니다. 해당 컨트롤은 UI 설정에 속합니다.
- 본체:
  - 데스크탑: 고정 4x3 월 카드 그리드(`desktopYearGridColumnCount = 4`),
  - 월 제목 색상은 고정 빨간색 대신 강조 토큰(`monthTitleColor = LV.Theme.accent`)을 사용합니다.
  - 데스크톱 월 카드는 `desktopResponsiveScale`를 사용하여 반응형으로 스케일링되며, 그리드 간격/카드 패딩/데이셀 크기가 창 크기에 따라 변경됩니다 (`yearGridSpacing`, `monthCardPadding` , `monthSectionSpacing` , `monthTitlePixelSize` ),
  - 주중/요일 숫자 라벨은 이제 `LV.Theme.textBody`를 통해 라우팅되고 `Font.Medium`로 유지됩니다.
  - 카드당 평일 헤더 행,
  - 월별 42셀 일 그리드입니다.
  명명된 `LV.Theme` 토큰 또는 토큰 구성을 통해.

<a id="lvrsqml-standard-alignment"></a>

## LVRS/QML 표준 정렬
- 중첩된 대리인이 명시적 바인딩 범위를 통해 외부 ID에 액세스할 수 있도록 `pragma ComponentBehavior: Bound`를 활성화합니다.
- 리피터 위임자는 `required property var modelData` 및 ID -qualified 매핑( `monthCard.modelData` , `dayCell.modelData` )을 사용하여 비정의 컨텍스트 읽기 대신 사용합니다.
- 이렇게 하면 달력 페이지가 작업 공간에서 사용되는 더 엄격한 LVRS/QML 린트 계약과 호환됩니다.

<a id="board-extensions"></a>

## 보드 확장
- 매일 셀은 `inCurrentMonth` 조광 기능과 원형 오늘 하이라이트를 갖춘 경량 텍스트 렌더링을 사용합니다.
- 인접한 오버플로우 날짜는 이제 `Qt.darker(activeDayColor, 1.2)`를 사용하여 라벨 기본 색상으로 되돌아가는 대신 월별 날짜 텍스트보다 약 20%만큼 어두운 상태를 유지하면서도 읽을 수 있습니다.
- 연간 뷰는 월·일 컨텍스트를 위한 고밀도 탐색 표면을 유지하면서 `YearCalendarController`의 보드 데이터 계약을 유지합니다.
- `YearCalendarController::focusToday()`는 표시된 연도를 활성 달력 시스템에 맞춥니다.
- 이제 월 제목을 탭하면 해당 카드의 첫 번째 날짜를 선택한 날짜로 사용하여 일치하는 월 오버레이를 요청합니다.
- 이제 일 탭은 탭한 날짜에 대한 월 오버레이를 요청하므로 인접한 오버플로 일이 실제 달로 전달됩니다.

<a id="tests"></a>

## 테스트

- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 헤더 이전/오늘/다음 작업은 선택기 행을 제거한 후에도 계속 작동해야 합니다.
    - 월 제목을 탭하면 콘텐츠 화면이 연도 보기에서 해당 월 보기로 전환되어야 합니다.
    - 월 내 또는 인접 월의 날짜를 탭하면 해당 탭된 날짜에 대한 월 보기가 열리고 선택된 ISO 날짜가 보존됩니다.
    - 3월에 표시된 `2/28`와 같은 인접한 오버플로우 날짜는 순수 검정/기본 라벨 텍스트가 아니라 테마에서 파생된 흐리게 표시된 색상으로 렌더링되어야 합니다.

<a id="collaborators"></a>

## 협력자
- `src/app/calendar/CalendarBoardStore.hpp/.cpp`
- `src/app/models/calendar/YearCalendarController.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
