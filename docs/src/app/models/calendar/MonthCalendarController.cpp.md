# `src/app/models/calendar/MonthCalendarController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 월 및 선택한 날짜 새로 고침이 이제 `ICalendarBoardStore`를 관찰합니다.
- 공유 월 그리드 빌더는 이제 정규 `displayedYear/displayedMonth` 상태와 월 페이저에서 사용되는 지속적인 `pagerMonthModels` 속성를 모두 지원합니다.
- 각 달마다 `dayModel`는 이제 해당 ISO 날짜에 대한 해결된 `entries` 배열을 집계 수와 함께 전달하므로, 월 그리드가 부수 효과만으로 재쿼팅하는 대신 재구성된 프로젝션에서 직접 노트/이벤트 칩을 렌더링할 수 있습니다.
- `requestMonthView(...)`는 이제 추가 한 달 동안 재구축을 강요하지 않고 훅/트레이싱 신호를 방출합니다. 월 데이터는 이미 세터/스토어에 의해 구동되는 상태 변화에 의해 소유되므로, 월 표면을 열 때 QML에서 추가 페이지 열기 재계산이 발생하지 않습니다.
- 따라서 월 보기는 이제 상위 공급 측 스토어/컨트롤러 신호 흐름이 올바르게 작동하는지 의존합니다: 프로젝트된 노트 캐시 업데이트는 `CalendarBoardStore::entriesChanged` 를 통해 도착해야 하며, 늦은 UI -트리거된 재빌드를 통해 도착해서는 안 됩니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 표준 현재 월 모델과 `pagerMonthModels[1]`는 동일한 월/연도를 설명해야 합니다.
    - 개시 월 보기에서는 올바른 월 그리드가 나타나기 전에 `requestMonthView(...)`가 프로젝션을 재구성할 필요가 없어야 합니다.
    - 사용자가 `Today`를 누르지 않은 경우에도 시작/지연된 라이브러리 로드 후에도 월간 메모 칩이 계속 나타나야 합니다.
    - 인접 월 호출기 데이터는 `shiftMonth(...)` 및 `focusToday()` 이후 동기화를 유지해야 합니다.
