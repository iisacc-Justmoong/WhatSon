# `src/app/models/hierarchy/preset/PresetHierarchyController.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 사이드바에서 사용되는 사전 설정된 계층 구조 컨트롤러를 정의합니다. 사전 설정된 분류법을 QML 친화적인 항목 모델로 노출하고 LVRS 계층 구조 뷰에 필요한 변형 및 확장 후크를 선언합니다.

<a id="public-contract"></a>

## 공공 계약

- `itemModel`, `hierarchyModel`, `selectedIndex`, `itemCount` 및 로드 상태 속성을 게시합니다.
- 기본 계층 구조 컨트롤러 계약 외에 이름 변경, 생성, 삭제 및 확장 인터페이스를 구현합니다.
- `setPresetNames(...)`를 통해 직접 사전 설정 이름 업데이트를 허용합니다.
- `applyRuntimeSnapshot(...)`를 통해 런타임 로더 업데이트를 허용합니다.
- `requestControllerHook()`를 디스크에서 `Preset.wspreset`를 다시 읽을 수 있는 파일 기반 새로 고침 훅으로 노출합니다.
- 모든 상속된 기능 메서드에 명시적인 `override` 마커를 사용하여 시그니처 드리프트가 즉시 포착되고 빌드가 경고‐클린 상태를 유지합니다.

<a id="state-rules"></a>

## 주 규칙

- `m_presetNames`는 표준 사전 설정 목록입니다.
- `m_items`는 렌더링된 계층 구조 행을 포함하고 `expanded` 상태를 저장합니다.
- `m_presetFilePath`는 지속성 돌연변이에 사용되는 `Preset.wspreset` 파일을 가리킵니다.

<a id="refresh-constraints"></a>

## 제약 조건 새로 고침

- 변경되지 않은 런타임 스냅샷은 계층 구조 행을 다시 작성해서는 안 됩니다.
- 변경된 스냅샷은 안정적인 사전 설정 행 키를 통해 선택과 확장을 모두 유지해야 합니다.
- 현재 표시되는 행을 삭제하지 않고 로드 상태가 변경되면 로드 실패가 표면화되어야 합니다.
