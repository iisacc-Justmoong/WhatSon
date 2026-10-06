# `src/app/qml/view/panels/HierarchySidebarLayout.qml`

<a id="role"></a>

## 역할
이 구성 요소는 일반 사이드바 보기와 런타임 계층 구조 라우팅 레이어 사이의 어댑터입니다.

4 개의 중요한 작업을 수행합니다.
- 현재 활성 계층 도메인을 확인합니다.
- `SidebarHierarchyController`에서 일치하는 도메인 컨트롤러를 선택합니다.
- 2 개의 시각 사이드바에 사용되는 브릿지 객체를 인스턴스화합니다.
- C++에서 바닥글 및 확장 결정을 유지하는 사이드바 상호 작용 정책 컨트롤러를 인스턴스화합니다.

<a id="binding-strategy"></a>

## 바인딩 전략
`resolvedHierarchyController`는 먼저 `sidebarHierarchyController.hierarchyControllerForIndex(currentHierarchy)`에서 확인된 다음 `sidebarHierarchyController.resolvedHierarchyController`를 통해 대체됩니다. 이전에 확인된 공급자 개체보다 약간 빠릅니다.

<a id="hosted-bridges"></a>

## 호스팅 브리지
- `HierarchyDragDropBridge`: 재정렬 및 메모 드롭 브리지.
- `HierarchyInteractionBridge`: 이름 바꾸기, 생성, 삭제 및 확장 브리지.
- `SidebarHierarchyInteractionController` : 푸터 디스패치, 중복 트리거 병합, 그리고 활성 계층 인덱스에 바인딩된 확장 상태 정책 컨트롤러.

<a id="layout-notes"></a>

## 레이아웃 참고 사항
- 공유 도구 모음 프레임 너비는 이제 원시 대신 `LV.Theme.inputMinWidth + LV.Theme.gap20`를 통해 확인됩니다.

<a id="why-this-file-is-important"></a>

## 이 파일이 중요한 이유
이는 저장소의 "one hierarchy type, one dedicated controller" 규칙이 런타임에서 도메인을 전환할 수 있는 시각적 사이드바로 변환되는 QML 이음새입니다.
