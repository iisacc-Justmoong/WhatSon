# `src/app/models/detailPanel/NoteDetailPanelController.hpp`

<a id="responsibility"></a>

## 책임
`NoteDetailPanelController`는 노트 받침 디테일 패널용으로 QML에 등록된 콘크리트 런타임 유형입니다.

<a id="structure"></a>

## 구조
- 동작을 변경하지 않고 `DetailPanelController`를 상속합니다.
- 런타임가 별도의 `ResourceDetailPanelController`와 함께 명시적으로 노트 스코프된 디테일 컨트롤러를 마운트할 수 있도록 존재합니다.

<a id="runtime-role"></a>

## 런타임 역할
- `main.cpp`는 모든 비자원 계층 구조에 대해 `NoteDetailPanelController`를 인스턴스화합니다.
- `DetailPanel.qml`는 해당 개체를 `NoteDetailPanel.qml`로 라우팅합니다.
