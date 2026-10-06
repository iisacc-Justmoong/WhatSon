# `src/app/qml/view/panels/detail/DetailPanelHeaderToolbarButton.qml`

<a id="responsibility"></a>

## 책임
`DetailPanelHeaderToolbarButton.qml`는 세부 도구 모음에서 사용되는 버튼별 대리자입니다. 검사/디버깅을 위해 Figma 개체 이름과 노드 ID를 유지하면서 도구 모음 사양 데이터를 `LV.IconButton`에 적용합니다.

<a id="contract"></a>

## 계약
- 루트 유형: `LV.IconButton`
- 크기 소스: LVRS `IconButton` 기본 측정항목(로컬 `20x20` 클램프 없음)
- 선택한 톤: `LV.AbstractButton.Default`
- 선택되지 않은 톤: `LV.AbstractButton.Borderless`

<a id="data-inputs"></a>

## 데이터 입력
- `buttonSpec.iconName`
- `buttonSpec.iconSource`
- `buttonSpec.objectName`
- `buttonSpec.figmaNodeId`
- `buttonSpec.selected`
- `buttonSpec.stateValue`

<a id="behavior"></a>

## 행동
- 유효하지 않거나 숫자가 아닌 `stateValue`는 무시되고 기존 보기-후크 경로를 통해 기록됩니다.
- 유효한 클릭은 `stateClickRequested(nextState)`를 방출합니다.
- 대리자는 축소된 `20x20` 프레임을 강제로 적용해서는 안 됩니다. 그래야 세부 헤더가 플랫폼에 적합한 버튼 크기를 상속할 수 있습니다.
