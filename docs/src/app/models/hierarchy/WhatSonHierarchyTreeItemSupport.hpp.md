# `src/app/models/hierarchy/WhatSonHierarchyTreeItemSupport.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 계층 지원 모듈에서 사용되는 반복되는 트리 항목 돌연변이 도우미를 중앙 집중화합니다.

- 깊이 관계에서 `showChevron`를 다시 계산합니다.
- 다음 `FolderN` 시퀀스 생성
- 공유 유효성 검사를 사용하여 항목 라벨 이름 바꾸기
- 버킷 헤더 행 감지
- 단순 또는 중첩 폴더 행 생성
- 선택한 하위 트리를 삭제하고 다음 선택 색인을 정규화합니다.

<a id="public-helpers"></a>

## 공공 도우미

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `createFlatHierarchyFolder(...)`
- `createNestedHierarchyFolder(...)`
- `deleteHierarchySubtree(...)`

<a id="variants"></a>

## 변형

2 삽입 전략은 의도적으로 분리됩니다:

- `createFlatHierarchyFolder(...)`: 선택한 행 뒤에 루트 수준 폴더를 추가하거나 삽입합니다.
- `createNestedHierarchyFolder(...)` : 선택한 서브트리 뒤에 하위 폴더를 삽입합니다. 상위 확장은 레거시 옵트인 플래그이며, 도메인 래퍼는 사용자가 명시적으로 확장 명령을 요청하지 않는 한 이를 비활성화된 상태로 두어야 합니다.

이렇게 하면 각 지원 헤더에서 중복된 변이 코드를 제거하는 동시에 도메인 래퍼를 작게 유지합니다.

<a id="consumers"></a>

## 소비자

이 도우미는 다음에 의해 다시 내보내집니다.

- `ProjectsHierarchyControllerSupport.hpp`
- `EventHierarchyControllerSupport.hpp`
- `BookmarksHierarchyControllerSupport.hpp`
- `ProgressHierarchyControllerSupport.hpp`
- `PresetHierarchyControllerSupport.hpp`
