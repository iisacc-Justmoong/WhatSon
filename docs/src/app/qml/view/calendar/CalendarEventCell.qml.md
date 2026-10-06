# `src/app/qml/view/calendar/CalendarEventCell.qml`

<a id="role"></a>

## 역할
`CalendarEventCell.qml`는 월간, 주간 및 일일 달력 보기에 사용되는 재사용 가능한 공유 이벤트 칩입니다. 이는 월간 이벤트 칩 설계 참조(Figma 노드 `227:9429`)에서 파생되었으며 크로스 뷰 사용을 위해 일반화되었습니다.

<a id="public-qml-contract"></a>

## 퍼블릭 QML 계약
- `property string label`
- `property int backgroundType` (`backgroundDefault` / `backgroundColored`)
- `property color defaultBackgroundColor`
- `property color coloredBackgroundColor`
- `property color textColor`
- `property int cornerRadius`
- `property int horizontalInset`
- `property int verticalInset`
- `property int labelPixelSize`
- `property int labelWeight`
- `property bool interactive`
- `signal activated`

<a id="render-rules"></a>

## 렌더링 규칙
- 기본 크기: `height` 기본값은 `LV.Theme.iconSm`입니다(발신자가 재정의할 수 있음).
- 반경: `cornerRadius`의 기본값은 `LV.Theme.radiusSm`입니다.
- 삽입: `horizontalInset` / `verticalInset` 기본값은 `LV.Theme.gap8` / `LV.Theme.gap2`입니다.
- 글꼴: `labelPixelSize`는 `LV.Theme.textBody`를 통해 기본값을 가지며 `labelWeight`는 `Font.Medium`로 유지됩니다.
- 텍스트 색상의 기본값은 `LV.Theme.bodyColor`입니다.
- 배경:
  - 기본값: `defaultBackgroundColor`(`LV.Theme.panelBackground08`)
  - 컬러: `coloredBackgroundColor` (`LV.Theme.primary`)
- 상호작용:
  - 내부 `TapHandler`는 `interactive == true`인 경우에만 `activated()`를 방출합니다.
  - 기본 계약은 수동 상태로 유지되므로 노트 칩이 아닌 칩을 실수로 클릭할 수 없게 되지 않습니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 패시브 이벤트 칩은 기본적으로 클릭할 수 없는 상태로 유지되어야 합니다.
    - 대화형 노트 칩은 마우스 클릭과 터치 탭 모두에서 `activated()`를 방출해야 합니다.

<a id="collaborators"></a>

## 협력자
- `src/app/qml/view/calendar/MonthCalendarDayCell.qml`
- `src/app/qml/view/calendar/WeekCalendarPage.qml`
- `src/app/qml/view/calendar/DayCalendarPage.qml`
