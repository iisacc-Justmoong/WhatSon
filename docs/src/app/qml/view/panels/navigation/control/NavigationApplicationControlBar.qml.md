# `src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml`

<a id="role"></a>

## 역할
`NavigationApplicationControlBar.qml`는 공유 탐색 모음에 대한 애플리케이션 수준 후행 제어 클러스터를 소유합니다.


<a id="compact-variants"></a>

## 소형 변형
이 파일은 2 개의 콤팩트 변형을 노출합니다.
- 계층/편집자 제어 경로: 내장된 LVRS 체브론 표시기가 포함된 `toolwindowtodo` 메뉴 트리거와 `compactDetailPanelVisible == true` 시 선택적 오른쪽 가장자리 `columnIndex` 상세 페이지 버튼이 포함됩니다.
- 노트 목록 경로: `sortByType`, `cwmPermissionView` 및 동일한 `toolwindowtodo` 메뉴 트리거


<a id="menu-ownership"></a>

## 메뉴 소유권
이 파일은 2 개의 컨텍스트 메뉴를 소유합니다.
- 계층/제어 경로용 `applicationControlContextMenu`
- 컴팩트한 메모 목록 경로를 위한 `noteListApplicationControlContextMenu`

두 메뉴 모두 트리거의 오른쪽 하단 모서리에서 `openFor(button, button.width, button.height + menuYOffset)`를 통해 고정됩니다.

<a id="desktop-composition"></a>

## 데스크탑 구성
데스크탑 행은 Figma 하위 순서를 유지합니다.
1. `NavigationAppControlBar`
2. `NavigationExportBar`
3. `NavigationAddNewBar`
4. `NavigationPreferenceBar`

<a id="invariants"></a>

## 불변성
- 압축 계층 모드는 `generalprojectStructure` 메뉴 문자 모양으로 되돌아가면 안 됩니다.
- 컴팩트 노트 목록 모드는 `sort -> visibility -> todo menu` 순서를 유지해야 합니다.
- 메뉴 항목은 `keyVisible: false` 및 `showChevron: false`를 사용하는 작업 전용 항목으로 유지됩니다.

<a id="recent-updates"></a>

## 최근 업데이트
- `pragma ComponentBehavior: Bound`를 추가하여 컴팩트/전체 모드 중첩된 `Component` 브랜치가 `applicationControlBar` ID 멤버에 접근할 수 있도록 LVRS -standard 바운드 컴포넌트 범위에 접근할 수 있습니다. 복제 컨텍스트 메뉴 작업.
- 그 컴팩트한 디테일 버튼은 이제 훅 사유 `open-detail-page`를 방출하여, 이전의 접기/확장 오버레이 문구 대신 라우팅된 페이지 상호작용과 일치합니다.
- 메뉴 너비/y 오프셋 및 데스크톱 전체 행 간격이 이제 원시 `176/2/12px` 리터럴이 아니라 `LV.Theme.inputMinWidth - LV.Theme.gap4` 및 `LV.Theme.gap2/12`를 라우팅합니다.
