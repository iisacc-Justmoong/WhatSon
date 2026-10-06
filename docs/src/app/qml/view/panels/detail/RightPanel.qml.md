# `src/app/qml/view/panels/detail/RightPanel.qml`

<a id="responsibility"></a>

## 책임
`RightPanel.qml`는 Figma `RightPanel` 프레임(`155:4574`)용 외부 데스크탑 상세 패널 셸입니다. 패널 캔버스를 투명하게 유지하고 패널-뷰-모델 후크 진입점을 유지하며 `DetailPanel.qml`를 통해 실제 디테일 구성을 마운트합니다.

<a id="visual-contract"></a>

## 시각적 계약
- Figma 루트 프레임 ID: `155:4574`
- 루트 `objectName`: `RightPanel`
- 기본/최소 패널 너비는 이제 명명된 `LV.Theme` 너비/간격/획 토큰 구성을 통해 확인됩니다.
- 하위 `DetailPanel`가 래퍼를 채우므로 상위 레이아웃 크기 조정이 최종 너비를 제어합니다.

<a id="runtime-notes"></a>

## 런타임 노트
- 파일은 의도적으로 얇게 유지됩니다.
- 세부 상태를 소유하지 않으며, 데스크톱 셸의 나머지 부분에서 이미 사용되는 공유 패널 래퍼 패턴을 통해 `calendarDetailActive`와 라이프사이클 가시성을 전달합니다.
- `requestViewHook(reason)`는 여전히 `panelControllerRegistry.panelController("detail.RightPanel")`를 통해 라우팅됩니다.

<a id="integration"></a>

## 통합
- 상위 래퍼: `src/app/qml/view/panels/DetailPanelLayout.qml`
- 하위 구성: `src/app/qml/view/panels/detail/DetailPanel.qml`
