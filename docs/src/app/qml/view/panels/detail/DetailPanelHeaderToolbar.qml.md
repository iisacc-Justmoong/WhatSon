# `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml`

<a id="responsibility"></a>

## 책임
`DetailPanelHeaderToolbar.qml`는 Figma 프레임 `155:4575`에 대한 6개 버튼 세부 도구 모음을 렌더링합니다. 도구 모음 사양을 정규화하고 확인된 버튼 행에서 크기를 조정하며 버튼이 활성화되면 `detailStateChangeRequested(int)`를 내보냅니다.

<a id="visual-contract"></a>

## 시각적 계약
- 루트 `objectName`: `DetailPanelHeaderToolbar`
- Figma 노드 ID: `155:4575`
- 프레임 크기: 내부 `Row` 암시적 크기에서 파생됨
- Inter 버튼 간격: `LV.Theme.gap5`
- 파일은 루트 범위에서 `LV.Theme`를 읽기 때문에 `LVRS 1.0 as LV`를 직접 가져와야 합니다.
- 도구 모음은 상위 패널의 중앙에 위치하지만 이 파일은 내부 행 형상만 유지합니다.

<a id="delegate-contract"></a>

## 계약위임
각 대리자는 다음을 포함하는 정규화된 사양 개체를 받습니다.
- `figmaNodeId`
- `iconName`
- `objectName`
- `stateValue`
- `selected`

도구 모음 자체는 데이터 기반으로 유지되며 선택한 상태 값을 내보내는 것 이상의 상태 전환을 하드코딩하지 않습니다.
