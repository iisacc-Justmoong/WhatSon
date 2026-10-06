# `src/app/models/detailPanel/DetailPanelCurrentHierarchyBinder.hpp`

<a id="role"></a>

## 역할
`DetailPanelCurrentHierarchyBinder`는 활성 사이드바 계층 구조 컨텍스트를 `DetailPanelController`에 바인딩하는 컴포지션 루트 코디네이터입니다.

<a id="responsibilities"></a>

## 책임
- `IActiveHierarchyContextSource`를 관찰합니다.
- 현재 노트 목록 모델과 현재 계층 구조 디렉터리 확인자를 `DetailPanelController`에 푸시합니다.
- 활성 계층 컨텍스트가 사라지면 해당 바인딩을 완전히 지웁니다.

<a id="dependency-direction"></a>

## 의존성 방향
- `SidebarHierarchyController`가 아닌 사이드바 측 추상화 `IActiveHierarchyContextSource`에 따라 다릅니다.
- 세부 패널 컨텍스트 동기화를 위해 임시 람다 배선 없이 `main.cpp`를 유지합니다.
