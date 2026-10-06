# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyModel.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 `BookmarksHierarchyController` 내부에서 사용되는 형식화된 `BookmarksHierarchyItem` 구조체 및 아이콘 도우미를 유지합니다.

<a id="shared-model-contract"></a>

## 공유 모델 계약

- 더 이상 Qt 항목 모델 클래스를 선언하지 않습니다.
- QML/LVRS 방향 모델은 공유 `WhatSonHierarchyModel`입니다.
- 북마크 색상/아이콘 메타데이터는 입력된 구조체에 유지되며 컨트롤러 `depthItems()` 맵을 통해 게시됩니다.
