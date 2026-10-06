# `src/app/qml/view/panels/detail/ResourceDetailPanel.qml`

<a id="responsibility"></a>

## 책임
`ResourceDetailPanel.qml`는 리소스 계층 구조에 대한 전용 세부 열 표시 영역입니다.

<a id="current-state"></a>

## 현재 상태
- 지금은 뷰가 의도적으로 비어 있습니다.
- 이미 자체 `resourceDetailPanelController` 계약을 수락하고 있으므로, 리소스별 세부 사항인 UI를 나중에 주석 상세 표면으로 다시 확장하지 않고 추가할 수 있습니다.

<a id="contract"></a>

## 계약
- `property var resourceDetailPanelController`
- `signal viewHookRequested`
- `function requestViewHook(reason)`
