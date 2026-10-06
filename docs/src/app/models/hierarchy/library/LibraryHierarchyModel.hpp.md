# `src/app/models/hierarchy/library/LibraryHierarchyModel.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 `LibraryHierarchyController` 내부에서 사용되는 형식화된 `LibraryHierarchyItem` 구조체와 `libraryHierarchyIconName(...)` 도우미를 유지합니다.

<a id="shared-model-contract"></a>

## 공유 모델 계약

- 더 이상 Qt 항목 모델 클래스를 선언하지 않습니다.
- QML/LVRS 방향 모델은 공유 `WhatSonHierarchyModel`입니다.
- 라이브러리별 행 아이덴티티는 구조체에 남아 있습니다: 시스템 버킷은 예약된 아이덴티티를 유지하고 일반 폴더는 `folderUuid` / `folderPath`를 전달하므로 컨트롤러 직렬화는 안정적인 `LV.Hierarchy` 노드 키를 생성할 수 있습니다.
