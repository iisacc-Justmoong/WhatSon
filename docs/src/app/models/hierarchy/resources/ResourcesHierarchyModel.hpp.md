# `src/app/models/hierarchy/resources/ResourcesHierarchyModel.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 `ResourcesHierarchyController` 내부에서 사용되는 형식화된 `ResourcesHierarchyItem` 구조체와 `resourcesHierarchyIconName(...)` 도우미를 유지합니다.

<a id="shared-model-contract"></a>

## 공유 모델 계약

- 더 이상 Qt 항목 모델 클래스를 선언하지 않습니다.
- QML/LVRS 방향 모델은 공유 `WhatSonHierarchyModel`입니다.
- `kind`, `bucket`, `type`, `format`와 같은 리소스 분류 필드와 리소스 경로는 타입이 지정된 구조체에 남아 있어 컨트롤러가 도메인별 뷰 모델을 추가하지 않고도 리소스 계층 노드를 직렬화할 수 있습니다.
