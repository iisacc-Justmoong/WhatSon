# `src/app/models/sidebar/HierarchyControllerProvider.hpp`

<a id="role"></a>

## 역할
`HierarchyControllerProvider`는 계층 도메인 인덱스를 전용 도메인 컨트롤러 및 메모 목록 모델로 확인합니다.

<a id="current-shape"></a>

## 현재 형태
- 계층 구조 도메인당 하나의 하드 코딩된 구조체 필드 대신 `Mapping { hierarchyIndex, controller }` 항목을 사용합니다.
- `HierarchySidebarDomain.hpp`로 입력된 인덱스 정규화된 레지스트리에 매핑을 저장합니다.
- 새로운 멤버 변수나 새로운 계층 도메인마다 스위치 브랜치를 요구하지 않고 확장을 위해 해결 경로를 열어 두십시오.

<a id="boundary"></a>

## 경계
- 컴포지션 루트는 여전히 구체적인 등록을 소유하고 있습니다.
- 해당 라인 아래의 소비자는 `IHierarchyControllerProvider`에만 의존합니다.
