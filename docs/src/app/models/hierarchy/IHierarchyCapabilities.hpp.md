# `src/app/models/hierarchy/IHierarchyCapabilities.hpp`

<a id="role"></a>

## 역할
이 헤더는 선택 계층 기능 인터페이스를 수집합니다.

<a id="recent-addition"></a>

## 최근 추가
- `IHierarchyReorderCapability`는 전체 노드 재오더 재생과 선택적인 직접 이동 이벤트 헬퍼를 제공합니다. 사이드바 드래그 경로는 `LV.Hierarchy.model`의 전체 노드 재생을 사용하며, 구체적인 컨트롤러는 명시적인 목표 이동 호출자를 위해 여전히 `applyHierarchyMove(...)`를 노출할 수 있습니다.
- `ILibraryNoteMutationCapability`는 이제 노트 생성, 폴더 지우기, 노트 삭제를 소규모 협업 계약으로 정의합니다.
- `LibraryNoteMutationController`는 전체 `LibraryHierarchyController` 유형 대신 이 기능을 사용합니다.

<a id="expansion-contract"></a>

## 확장 계약
`IHierarchyExpansionCapability`는 `HierarchyInteractionBridge`에서 조사하는 공개 기능으로 유지됩니다. 구체적인 컨트롤러는 공유 행 유효성 검사 및 상태 변형을 `IHierarchyController`의 보호된 도우미에 위임한 다음 도메인별 모델 동기화 또는 지속성 후속 조치만 수행하여 해당 기능을 구현해야 합니다.
