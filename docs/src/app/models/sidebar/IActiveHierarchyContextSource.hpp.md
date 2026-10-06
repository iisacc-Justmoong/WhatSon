# `src/app/models/sidebar/IActiveHierarchyContextSource.hpp`

<a id="role"></a>

## 역할
`IActiveHierarchyContextSource`는 활성 계층 컨트롤러 및 활성 노트 목록 모델을 사용하여 `IActiveHierarchySource`를 확장합니다.

<a id="why-it-exists"></a>

## 그것이 존재하는 이유
- 시작/ 런타임 코드는 활성 인덱스만 필요할 경우 더 작은 `IActiveHierarchySource` 계약에 계속 의존할 수 있습니다.
- 디테일 패널 바인더와 같은 소비자는 구체적인 `SidebarHierarchyController` 유형 대신 이보다 풍부한 인터페이스에 의존할 수 있습니다.

<a id="exposed-contract"></a>

## 노출된 계약
- `activeHierarchyIndex()`
- `activeHierarchyController()`
- `activeNoteListModel()`
- `activeBindingsChanged()`, `activeHierarchyControllerChanged()`, `activeNoteListModelChanged()`
