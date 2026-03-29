# `src/app/file/hierarchy/resources/WhatSonResourcesHierarchyStore.cpp`

## Responsibility

리소스 목록 저장소는 이제 raw asset path가 아니라 `.wsresource` 패키지 경로를 보존한다.

## Normalization

`setResourcePaths(...)`는 입력값을 trim하는 것에서 끝나지 않고 경로 정규화도 수행한다.
즉 저장소가 가지는 리스트는:

- 비어 있지 않고
- 중복이 없고
- slash form이 정리된 패키지 경로 목록

이라는 조건을 유지한다.

## Persistence

`writeToFile(...)`는 `WhatSonResourcesHierarchyCreator`를 통해 `Resources.wsresources`를 새 object-array 포맷으로 기록한다.
