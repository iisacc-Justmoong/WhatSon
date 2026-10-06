# `src/app/models/hierarchy/projects/ProjectsHierarchyModel.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 `ProjectsHierarchyController` 내부에서 사용되는 형식화된 `ProjectsHierarchyItem` 구조체 및 아이콘 도우미를 유지합니다.

<a id="shared-model-contract"></a>

## 공유 모델 계약

- 더 이상 Qt 항목 모델 클래스를 선언하지 않습니다.
- QML/LVRS 방향 모델은 공유 `WhatSonHierarchyModel`입니다.
- 중첩된 프로젝트 깊이, 확장 및 드래그 플래그는 컨트롤러에 의해 `LV.Hierarchy` 노드 맵으로 직렬화됩니다.
