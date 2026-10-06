# `src/app/models/detailPanel/ResourceDetailPanelController.hpp`

<a id="responsibility"></a>

## 책임
`ResourceDetailPanelController`는 리소스 계층 구조를 위한 전용 세부 패널 컨트롤러입니다.

<a id="exported-contract"></a>

## 수출된 계약서
- `resourceListModel`
- `resourceContextLinked`
- `currentResourceEntry`
- `setCurrentResourceListModel(QObject*)`

<a id="notes"></a>

## 메모
- 해당 유형은 일치하는 `ResourceDetailPanel.qml` 표면이 현재 비어 있기 때문에 의도적으로 좁은 계약을 유지합니다.
- 이 클래스는 현재도 여전히 존재하므로, 리소스‐디테일 라우트가 노트 디테일에서 구조적으로 분산될 수 있으며, 노트별 선택자나 툴바 상태를 공유하지 않습니다.
