# `src/app/qml/view/calendar/MonthCalendarGridSurface.qml`

<a id="role"></a>

## 역할

`MonthCalendarGridSurface.qml`는 `MonthCalendarPage.qml`에서 사용하는 재사용 가능한 월 그리드 렌더러입니다.

<a id="responsibilities"></a>

## 책임

- 한 달 예측을 위해 평일 헤더 밴드와 7열 날짜 그리드를 렌더링합니다.
- 표시되는 행 수를 계산하기 전에 42셀 투영에서 후행 모든 인접 월 행을 자릅니다.
- 월 전망이 이미 해당 칩을 보유하고 있을 때 `dayModel.entries`의 일일 엔트리 칩을 해결하고, 해당 페이로드가 없을 때만 `calendarController.entriesForDate(...)`로 되돌아갑니다.
- `dayModelForIndex(...)`를 통해 숫자 인덱스로 보이는 일 셀을 해결하여 재구성된 월 예측이 오래된 `modelData` 스냅샷이 잘못된 캘린더 날짜에 첨부되지 않도록 합니다.
- 수동 캘린더 이벤트와 예상 노트 수명주기 칩을 동일한 날짜 셀 표면에 유지하세요.
- 달력 히트 테스트를 통해 노트를 다시 열 수 있도록 각 예상 노트 칩의 `noteId`를 데이 셀 페이로드에 보존합니다.
- `selectedDateIso`를 각 `MonthCalendarDayCell`로 전달하면 선택한 날짜가 악센트 테두리 상태를 렌더링할 수 있습니다.
- 현재 날짜가 소프트 아웃라인 상태를 렌더링할 수 있도록 `dayModel.isToday`를 각 `MonthCalendarDayCell`로 전달합니다.
- 일 셀을 탭하면 `dateSelected(dateIso)`를 방출합니다.
- 데이 셀 내부의 노트 칩을 탭하면 `noteOpenRequested(noteId)`가 방출됩니다.

<a id="public-contract"></a>

## 공공 계약

- 입력:
    - `calendarController`
    - `monthProjection`
    - `selectedDateIso`
    - `weekdayHeaderHeight`, `weekdayCellHorizontalPadding`, `bodyLabelPixelSize`와 같은 크기 조정/스타일 소품
- 신호:
    - `dateSelected(string dateIso)`
    - `noteOpenRequested(string noteId)`
    - `viewHookRequested(string reason)`
- 후크 전달자:
    - `requestViewHook(reason)`

<a id="notes"></a>

## 메모

- 이 구성 요소는 의도적으로 월 그리드 렌더링과 일 셀 상호 작용만 소유합니다.
- 월별 탐색 및 수평 페이징은 `MonthCalendarPage.qml`에 유지됩니다.
- 주중 헤더 높이, 왼쪽 패딩 및 레이블 크기가 이제 `LV.Theme.controlHeightMd + LV.Theme.gap3`, `LV.Theme.gap12` 및 `LV.Theme.textBody`를 통해 해결되어 월별 그리드가 고정된 `12/39px` 값 대신 LVRS 밀도 스케일을 따릅니다.
- 이제 투영된 메모 칩은 캘린더 강조 색상을 사용하고 수동 이벤트 칩은 기본 이벤트 스타일을 유지합니다.

<a id="tests"></a>

## 테스트

- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 42-셀 투영은 여섯 번째 행에 현재 월 날짜가 비어 있을 때, 월 외 행을 아래로 축소하여 5행 그리드로 변환해야 합니다.
    - 데이 탭은 계속해서 항목 칩을 해결하고 선택한 ISO 날짜를 내보내야 합니다.
    - `dayModel.entries`에 노트 수명 주기 항목이 포함된 재구성된 월 프로젝션은 해당 `MonthCalendarDayCell`에서 가시적인 칩을 표시해야 합니다.
    - `buildEntryCellModels(...)`는 수동 이벤트와 예상 메모 모두에 대해 눈에 보이는 칩 페이로드를 반환해야 합니다.
    - `allDay`로 표시된 예상 메모 항목은 칩 라벨에 `00:00`를 추가해서는 안 됩니다.
    - 투사된 노트 칩은 `noteId`를 `buildEntryCellModels(...)`를 통해 운반하고, 탭될 때 `noteOpenRequested(...)`를 방출해야 합니다.
    - 초기 월 그리드 재구축은 현재 월 날짜 (예: `2026-04-01`) 에서의 메모를 인접 월 비활성화 셀 (예: `2026-05-01`) 로 잘못 배치하지 않아야 합니다.
    - `selectedDateIso`의 표시 셀은 선택 항목 변경 후 선택한 악센트 테두리를 렌더링해야 합니다.
    - 오늘 ISO 날짜에 표시되는 셀은 월 예측이 다시 작성될 때 소프트 테두리를 유지해야 합니다.
