# `src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml`

<a id="responsibility"></a>

## 책임
`NavigationApplicationEditBar.qml`는 탐색 모드가 `Edit`일 때 오른쪽 응용 프로그램 도구 모음 클러스터를 렌더링합니다.

Figma 노드 매핑은 다음과 같습니다.
- `149:4102` `ApplicationEditBar`
- 하위 주문: `CalendarBar -> AddNewBar -> PreferenceBar`

<a id="layout-contract"></a>

## 레이아웃 계약
- 루트 개체: `Item`
- 공개 모드 스위치: `property bool compactMode`
- 전체 모드(`compactMode: false`):
  - `NavigationApplicationCalendarBar`
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
- 전체 모드 `NavigationApplicationCalendarBar` 훅이 이제 `requestViewHook(reason)` 로 다시 전달되므로, 아이콘 모드와 컴팩트 메뉴 모드에서 연도 캘린더 클릭은 동일한 이유 파이프라인을 공유합니다.
- 컴팩트 메뉴는 전체 모드 기본 도구를 반영합니다.
  - 작업 및 달력 항목(`Task`, `Daily Calendar`, `Weekly Calendar`, `Monthly Calendar`, `Yearly Calendar`)
  - 새 파일
  - 환경설정

<a id="panel-controller-binding"></a>

## 패널 컨트롤러 바인딩
- 패널 키: `navigation.NavigationApplicationEditBar`
- 바인딩: `panelControllerRegistry.panelController("navigation.NavigationApplicationEditBar")`

<a id="notes"></a>

## 메모
- 전체 모드 자식 프레임은 `navigation/`의 공유 래퍼를 재사용하여 보기 및 편집 모드에서 중복된 모드 로컬 래퍼 파일을 방지합니다. `nodesnewFolder`는 `NavigationBarLayout.qml`의 폴더 추가 버튼입니다. 일관된 메뉴 어포던스 패턴입니다.
- 컴팩트 트리거는 제어 모드에서 사용되는 공유 메뉴 버튼 패딩 계약을 따릅니다: `left=2`, `right=4`, `top=2`, `bottom=2`, `spacing=0`.
- 컴팩트 메뉴 너비/y-오프셋은 이제 `LV.Theme.inputMinWidth + LV.Theme.gap16` 및 `LV.Theme.gap2`를 통해 라우팅됩니다.
- 콤팩트 편집기 라우트는 이제 Figma 노드 `193:6606`에서 전용 오른쪽 가장자리 `columnIndex` 상세 버튼을 렌더링하며, 데스크톱 환경설정 바에서 동일한 `toggleDetailPanelRequested` 신호 경로를 사용합니다. 컨텍스트 메뉴 항목입니다.
- 콤팩트 디테일 버튼이 이제 훅 사유 `open-detail-page`를 발생시켜, 이전의 접기/확대 오버레이 문구 대신 라우팅된 페이지 동작과 일치합니다.
- `pragma ComponentBehavior: Bound`가 활성화되어 컴팩트/전체 중첩된 `Component` 브랜치가 `applicationEditBar` id 멤버에 대한 비정격 경고 없이 접근할 수 있습니다.
