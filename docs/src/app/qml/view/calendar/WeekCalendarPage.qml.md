# `src/app/qml/view/calendar/WeekCalendarPage.qml`

<a id="role"></a>

## 역할
`WeekCalendarPage.qml` 는 주 캘린더를 3열 인라인 타임라인 표면으로 렌더링합니다. 주 표면은 이제 하나의 연속된 시간 스펙트럼을 유지합니다: 왼쪽 `Time` 헤더와 24 시간별 레이블은 고정되어 있고, 오른쪽의 일 열만 수평으로 이동합니다. 가시적인 뷰포트 는 여전히 3 일 열을 위한 크기를 갖지만, 수평 스와이프는 페이지 스냅 없이 인접한 날짜를 가로지릅니다. 페이지는 이제 두 번째 날짜 모델/캐시 레이어를 소유하지 않으며, 뷰포트 이동만 관리하고 타임라인 데이터 소유권을 `WeekCalendarController` 에 위임합니다.

<a id="view-contract"></a>

## 계약 보기
- 입력: `weekCalendarController`
- 출력 신호: `noteOpenRequested(string noteId)`
- 후크 신호: `viewHookRequested(string reason)`
- 후크 전달자: `requestViewHook(reason)`는 `weekCalendarController.requestWeekView(reason)`에 위임합니다.

<a id="ui-composition"></a>

## UI 구성
- 표면:
  - 루트 페이지는 `LV.Theme.accentTransparent`를 유지하므로 앱 배경이 표시됩니다.
- 헤더:
  - 공유 `CalendarTodayControl`(`Prev/Today/Next`)만 해당됩니다.
  - `Prev/Next`는 현재 초점을 맞춘 중간 날짜로부터 주 단위로 다시 중심을 잡는 반면, 가로로 긋기는 일별 순회를 처리합니다.
  - `Today`는 표면의 중심을 조정하여 실제 현재 날짜가 표시되는 날짜의 중간 열에 위치하도록 합니다.
- 본체:
  - 고정된 왼쪽 시간 비계(`Time` 헤더 + 24 시간 라벨) 1개,
  - 날짜 열만 소유하는 오른쪽의 수평 `Flickable` 하나,
  - 표시되는 뷰포트는 한 번에 3일 열에 해당하는 크기입니다.
  - 날짜-열 모델은 이제 `WeekCalendarController.timelineDayModels`에서 직접 제공됩니다.
  - 각 일일 모델은 이미 날짜별 항목 목록과 파생 메타데이터를 포함하고 있으므로, 페이지는 더 이상 로컬 `ListModel`, `dateEntriesCache` 또는 `buildDateModel(...)` 함수를 유지하지 않습니다.
  - 전용 날짜 헤더 행은 둘 다 동일한 수평 콘텐츠 표면에 있기 때문에 시간별 그리드와 정렬된 상태로 유지됩니다.
  - 일반 날짜 헤더와 시간 셀은 현재 주에 속하는지와 관계없이 `LV.Theme.accentTransparent`를 통해 투명하게 유지하므로 주 화면은 열별 그리드 채우기를 그리지 않는다,
  - 이번 주 강조는 이제 텍스트 어조와 기존 오늘 개요로 제한됩니다.
  - 스캐폴드는 사용 가능한 뷰포트 높이에서 `hourRowHeight`를 계산하므로 24 행은 나머지 영역(`00:00` 상단 슬롯, 하단 가장자리의 최종 슬롯)을 균등하게 채웁니다.
  - 3 일 열은 시간 열과 3 열 간격 이후 남은 너비를 나눕니다.
  - 가로 스크롤에는 스냅 동작이 없습니다.
- 중심일 하이라이트 반경은 이제 로컬 픽셀 상수 대신 `LV.Theme.gap20 + LV.Theme.strokeThin`를 통해 라우팅됩니다.

<a id="interactiondata-flow"></a>

## 상호작용/데이터 흐름
1. `Component.onCompleted`는 중앙에 배치된 레이지 날짜 창을 초기화하고, 현재 주를 열 때 주 시작 월요일 앵커 대신 실제 현재 날짜를 초기 포커스된 중간 열로 사용합니다.
2. 수평 이동은 오른쪽 날짜 열 표면만 업데이트합니다. 시간 비계는 움직이지 않습니다.
3. 니어 에지 액세스로 인해 지연 날짜 증가가 발생합니다.
   - 왼쪽 가장자리: `prependDates(...)` -> `WeekCalendarController.prependTimelineDates(...)`
   - 오른쪽 가장자리: `appendDates(...)` -> `WeekCalendarController.appendTimelineDates(...)`
4. 인접한 콘텐츠는 한 번에 하루씩 진행되므로 사용자는 `1-3`, `2-4`, `3-5` 등을 탐색할 수 있습니다.
5. Date-window 크기는 한계가 설정된 (`maxDateWindowSize`) 에서 `WeekCalendarController.trimTimelineWindow(...)` 로 제한되어 무한한 메모리 성장을 피합니다.
6. 프로그래밍 방식 중심 조정(`Prev`, `Next`, `Today`)은 주 앵커를 동기화하기 전에 대상 날짜를 중간 열에 정렬합니다.
7. 수평 이동 종료는 현재 집중된 중간 날짜를 월요일 시작 주로 정규화하고 `displayedWeekStartIso`를 동기화합니다.
8. 시간 슬롯 레이블은 컨트롤러 소유 요일 모델 `entries` 페이로드에서 파생되며 `slotSummary(...)`로 압축됩니다.
9. 시간 슬롯 항목 칩은 공유 `CalendarEventCell`(기본/항목 유형별 배경 색상)로 렌더링됩니다.
10. 직접 메모 열기는 눈에 보이는 슬롯 칩이 정확히 하나의 투사된 메모 항목을 나타내는 경우에만 활성화됩니다. 압축된 `title +N` 칩은 고유한 노트 타깃을 노출하지 않기 때문에 수동적인 상태를 유지합니다.

<a id="tests"></a>

## 테스트

- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
  - 주간 페이지에서는 `Time` 헤더와 24 시간 레이블을 고정하고 날짜 열만 수평으로 이동해야 합니다.
  - 주간 달력 콘텐츠 너비는 뷰포트 이내로 유지되어야 하며 유휴 상태의 정확히 3일 열을 표시해야 합니다.
  - 가로 스크롤은 페이지 스냅 없이 계속해서 움직여야 합니다.
  - 주 페이지는 컨트롤러 데이터 위에 두 번째 로컬 `ListModel` 또는 날짜별 항목 캐시를 유지해서는 안 됩니다.
  - 이번 주의 초기 페이지 열기는 주가 시작되는 월요일 대신 실제 현재 날짜를 중앙에 두어야 합니다.
  - `Today`를 클릭하면 일반 현재 주 머리글/시간 셀에 눈에 띄는 테두리가 표시되어서는 안 됩니다.
  - `Today`를 클릭하면 실제 현재 날짜가 표시되는 중간 날짜 열에 배치되어야 합니다.
  - 일반 헤더 및 시간 셀은 모든 날짜 열에 대해 투명하게 유지되어야 합니다.
  - 주간 페이지는 시간별 그리드 위에 전용 날짜 헤더 행을 렌더링해야 합니다.
  - 오늘은 날짜 헤더 행에서 시각적으로 구별되어야 합니다.
  - 정확히 하나의 메모 항목으로 뒷받침되는 주간 슬롯 칩은 클릭/탭 시 `noteOpenRequested(...)`를 방출해야 합니다.
  - 압축된 다중 항목 주간 슬롯 칩은 슬롯 UI가 항목별 적중 대상을 노출할 때까지 열 수 없는 상태로 유지되어야 합니다.

<a id="collaborators"></a>

## 협력자
- `src/app/models/calendar/WeekCalendarController.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
