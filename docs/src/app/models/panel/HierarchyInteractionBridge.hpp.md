# `src/app/models/panel/HierarchyInteractionBridge.hpp`

<a id="role"></a>

## 역할
이 헤더는 드래그가 아닌 상호 작용 명령에 대해 QML 계층 구조 보기에서 사용되는 브리지를 정의합니다.

이는 다음과 같은 질문에 답하는 기능 인식 표면에 일반 `QObject*` 계층 컨트롤러 입력을 적용합니다.
- 이 행의 이름을 바꿀 수 있나요?
- 이 계층 구조에서 폴더를 만들 수 있나요?
- 선택한 폴더를 삭제할 수 있나요?
- 행이 확장되거나 축소될 수 있나요?

<a id="why-it-accepts-qobject"></a>

## `QObject*`를 허용하는 이유
QML는 객체 참조를 쉽게 전달할 수 있지만 C++ 인터페이스 상속에 대해서는 알지 못합니다. 따라서 브리지는 원시 `QObject*`를 받아들이고 이를 `IHierarchyController`로 캐스팅한 다음 런타임에서 기능 인터페이스를 검색합니다.

<a id="public-surface"></a>

## 공공 표면
- 속성:
  - `hierarchyController`
  - `renameContractAvailable`
  - `createFolderEnabled`
  - `deleteFolderEnabled`
  - `viewOptionsEnabled`
- 호출 가능 항목:
  - `canRenameItem(...)`
  - `renameItem(...)`
  - `createFolder()`
  - `deleteSelectedFolder()`
  - `setItemExpanded(...)`
  - `setAllItemsExpanded(...)`

<a id="expected-consumer"></a>

## 예상 소비자
`SidebarHierarchyView.qml`는 이 브리지의 주요 소비자입니다.
