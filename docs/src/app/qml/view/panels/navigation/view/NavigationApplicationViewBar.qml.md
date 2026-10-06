# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml`

<a id="responsibility"></a>

## 책임
`NavigationApplicationViewBar.qml`는 탐색 모드가 `View`일 때 오른쪽 응용 프로그램 도구 모음 클러스터를 렌더링합니다.

Figma 노드 매핑은 다음과 같습니다.
- `149:4000` `ApplicationViewBar`
- 하위 주문: `ViewOptionBar -> ModeBar -> CalendarBar -> AddNewBar -> PreferenceBar`

<a id="layout-contract"></a>

## 레이아웃 계약
- 루트 개체: `Item`
- 공개 모드 스위치: `property bool compactMode`
- 전체 모드(`compactMode: false`):
  - `NavigationApplicationViewOptionBar`
  - `NavigationApplicationViewModeBar`
  - `NavigationApplicationViewCalendarBar`
  - `NavigationApplicationAddNewBar`
  - `NavigationApplicationPreferenceBar`
- 컴팩트 모드(`compactMode: true`):
  - `LV.IconMenuButton` 트리거 1개
  - `compactDetailPanelVisible == true`인 경우 선택적 `LV.IconButton` 세부 패널 트리거 1개
  - `Controls.Overlay.overlay`에 렌더링된 `LV.ContextMenu` 1개

<a id="interaction-contract"></a>

## 상호작용 계약
- `toggleDetailPanelRequested`를 노출하고 `NavigationPreferenceBar`에서 전달합니다.
- `viewHookRequested`를 노출하고 `panelController.requestControllerHook(reason)`를 통해 모드 수준의 훅 사유를 전달합니다.
- 전체 모드 보기 전용 자식 바(`ViewOptionBar`, `ModeBar`, `CalendarBar`)는 훅을 다시 `requestViewHook(reason)` 로 전달하므로, 전체 모드 아이콘과 콤팩트 메뉴 행이 하나의 이유 파이프라인을 공유합니다.
- 컴팩트 메뉴는 전체 모드 기본 도구를 반영합니다.
  - 옵션 보기(`Read Only`, `Wrap Text`, `Center View`, `Text To Speech`, `Paper Options`)
  - 보기 모드(`Center View Mode`, `Focus Mode`, `Presentation`)
  - 작업 및 달력 항목(`Task`, `Daily Calendar`, `Weekly Calendar`, `Monthly Calendar`, `Yearly Calendar`)
  - 새 파일
  - 환경설정

<a id="panel-controller-binding"></a>

## 패널 컨트롤러 바인딩
- 패널 키: `navigation.NavigationApplicationViewBar`
- 바인딩: `panelControllerRegistry.panelController("navigation.NavigationApplicationViewBar")`

<a id="notes"></a>

## 메모
- 전체 모드 자식 프레임은 이제 보기 전용 Figma 슬라이스를 `navigation/view/` 아래에 있는 전용 로컬 파일로 분할하고, `AddNewBar`와 `PreferenceBar`는 `navigation/` 아래에서 공유된 상태를 유지합니다.
- 보기 전용 캘린더 클러스터는 더 이상 공유 루트 `NavigationCalendarBar.qml`를 재사용하지 않으며, 이는 Figma `149:4001`가 복원된 작업 버튼에 대해 `validator`를 뷰 모드 `ApplicationViewBar`에서만 사용하기 때문입니다. `nodesnewFolder` 추가 폴더 버튼은 `NavigationBarLayout.qml`에서 제공됩니다. 일관된 메뉴 어포던스 패턴.
- 컴팩트 트리거는 제어 모드에서 사용되는 공유 메뉴 버튼 패딩 계약을 따릅니다: `left=2`, `right=4`, `top=2`, `bottom=2`, `spacing=0`.
- 컴팩트 메뉴 너비/y-오프셋은 이제 `LV.Theme.inputMinWidth + LV.Theme.gap16` 및 `LV.Theme.gap2`를 통해 라우팅됩니다.
- `Center View` 옵션은 LVRS `recursiveMethod` 를 통해 Figma `258:8039` 타겟/리테일 글리프를 유지하는 반면, `Center View Mode` 는 `258:7852` 에서의 별도 LVRS `singleRecordView` 화면 미리보기 글리프를 유지합니다.
- 컴팩트 편집기 라우트는 이제 `DetailPanelControlButton` 어포던스에서 전용 오른쪽 가장자리 `columnIndex` 상세 버튼을 렌더링하며, 로컬에서는 `detailPanelControlButton` 로 표시되고, 데스크톱 환경설정 바에서 사용되는 동일한 `toggleDetailPanelRequested` 신호 경로를 사용합니다. 컨텍스트 메뉴 항목.
- 콤팩트 디테일 버튼이 이제 훅 사유 `open-detail-page`를 발생시켜, 이전의 접기/확대 오버레이 문구 대신 라우팅된 페이지 동작과 일치합니다.
- `pragma ComponentBehavior: Bound`가 활성화되어 컴팩트/전체 중첩된 `Component` 브랜치가 `applicationViewBar` id 멤버에 대한 비정격 경고 없이 접근할 수 있습니다.

<a id="regression-checklist"></a>

## 회귀 체크리스트
- 하위 프레임 순서 `ViewOptionBar -> ModeBar -> CalendarBar -> AddNewBar -> PreferenceBar`를 유지합니다.
- 첫 번째 달력 컴팩트 메뉴 항목을 `validator`에 유지하고 레이블을 `Task`로 지정합니다.
- `recursiveMethod`에 `Center View` 옵션 아이콘을 유지합니다. 모드 수준 `singleRecordView` 문자 모양으로 다시 축소하지 마세요.
- 컴팩트 메뉴 섹션 순서를 전체 데스크탑 표시줄 순서에 맞게 유지하세요.
