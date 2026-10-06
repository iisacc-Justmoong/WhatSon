# `src/app/models/detailPanel/DetailHierarchySelectionController.hpp`

<a id="responsibility"></a>

## 책임
`DetailHierarchySelectionController`는 세부 패널 로컬 선택기 상태 개체입니다. 소스 계층 컨트롤러의 계층 항목을 미러링하지만 자체 `selectedIndex`를 소유하므로 세부 정보 패널이 암시적으로 사이드바 계층 구조 선택을 변경하거나 따르지 않습니다.

<a id="public-contract"></a>

## 공공 계약
- QML 콤보/컨텍스트 메뉴 바인딩을 위해 `hierarchyModel`를 노출합니다.
- `selectedIndex`를 세부 패널 로컬 선택 슬롯으로 노출합니다.
- 간단한 가용성 확인을 위해 `itemCount`를 노출합니다.
- `hierarchyModel`, `selectedIndex`, `hierarchyModelChanged()` 및 `selectedIndexChanged()`를 게시해야 하는 `sourceController` QObject를 허용합니다.
- 노출된 `hierarchyModel`에는 삽입된 선택기 소스가 이를 제공하기로 선택한 경우 원시 도메인 옵션보다 먼저 합성 파일 기반 지우기 항목이 포함될 수 있습니다.

<a id="invariants"></a>

## 불변성
- 소스 계층 구조 데이터는 변경 가능한 선택 상태로 공유되지 않고 복사됩니다.
- 초기 선택기 상태는 소스 선택에서 한 번 자체적으로 시드될 수 있습니다.
- 후속 소스 선택 변경 사항은 삽입된 세부 선택기 소스 개체에서 오는 경우에만 미러링됩니다.
- 후속 소스 계층 데이터 변경은 가능한 경우 안정적인 항목 키로 로컬 선택을 유지하면서 복사된 항목을 새로 고칩니다.
- 원시 사이드바 계층 구조 컨트롤러는 여전히 변경 가능한 선택기 소스로 직접 사용되지 않습니다. 파일 지원 선택기 소스 개체 뒤에 있는 옵션 공급자로 남아 있습니다.
