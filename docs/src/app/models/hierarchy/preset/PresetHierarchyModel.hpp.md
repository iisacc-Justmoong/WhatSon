# `src/app/models/hierarchy/preset/PresetHierarchyModel.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 `PresetHierarchyController` 내부에서 사용되는 형식화된 `PresetHierarchyItem` 구조체를 유지합니다.

<a id="shared-model-contract"></a>

## 공유 모델 계약

- 더 이상 Qt 항목 모델 클래스를 선언하지 않습니다.
- QML/LVRS 방향 모델은 공유 `WhatSonHierarchyModel`입니다.
- 사전 설정 이름은 컨트롤러에 의해 명명된 문자열 `LV.Hierarchy` 노드 맵으로 변환됩니다.
