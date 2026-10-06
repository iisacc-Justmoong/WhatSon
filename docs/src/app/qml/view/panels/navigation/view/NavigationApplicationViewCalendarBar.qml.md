# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewCalendarBar.qml`

<a id="responsibility"></a>

## 책임
`NavigationApplicationViewCalendarBar.qml`는 `ApplicationViewBar` 내부의 Figma `149:4001`에 대한 보기 전용 캘린더 클러스터를 소유하고 있습니다.

<a id="figma-mapping"></a>

## Figma 매핑
- 프레임: `149:4001` `CalendarBar`
- 버튼 순서:
  - `149:4002` `TaskButton` -> `validator`
  - `149:4003` `DailyCalButton` -> `newUIlightThemeSelected`
  - `149:4004` `WeeklyCalButton` -> `table`
  - `149:4005` `MonyhltCalButton` -> `pnpm`
  - `149:4006` `YearlyCalButton` -> `runshowCurrentFrame`

<a id="interaction-contract"></a>

## 상호작용 계약
- 루트 유형: `LV.HStack`
- `viewHookRequested(string reason)` 및 `requestViewHook(reason)`를 노출합니다.
- 복원된 가장 왼쪽 작업 버튼은 `view-open-task`를 표시합니다. 제거된 레거시 후크를 다시 도입하지 않습니다.

<a id="regression-checklist"></a>

## 회귀 체크리스트
- `taskButton`에 `validator` 아이콘을 유지하십시오. `toolWindowCheckDetails`로 돌아가지 마세요.
- 5개 버튼 순서를 유지하여 컴팩트 메뉴 패리티를 예측 가능하게 유지하세요.
