# `src/app/models/sidebar`

## Role
This directory contains the controller layer that coordinates which hierarchy domain is currently active in the sidebar.

It sits above concrete hierarchy domains but below the visual sidebar QML. Its job is routing and normalization, not persistence-heavy mutation.

## Important Types
- `SidebarHierarchyController`: exposes the active hierarchy index and resolves the active hierarchy and note-list models.
- `SidebarHierarchyInteractionController`: owns sidebar footer action dispatch, duplicate-action coalescing, chevron
  expansion state preservation, and expansion activation suppression for the visual hierarchy.
- `IActiveHierarchyContextSource`: exposes the active hierarchy index plus the active hierarchy/note-list binding
  snapshot for consumers that need more than activation alone.
- `IHierarchyControllerProvider` and `HierarchyControllerProvider`: map sidebar domain indices to dedicated hierarchy controllers.
- `HierarchySidebarDomain.hpp`: shared constants and index normalization helpers.

## Why This Layer Exists
The sidebar should not know how to discover the concrete controller for each domain. It should ask one coordinator for:
- the active domain index
- the active hierarchy controller
- the active note-list model

That coordination is exactly what this directory provides.

`HierarchyControllerProvider` now stores those bindings as index-addressable `Mapping` entries rather than one
hard-coded `Targets` struct field per domain, so the provider no longer needs a switch statement or one member per
hierarchy type just to resolve the active module.

Automated C++ regression coverage for this directory now lives in
`test/cpp/suites/*.cpp`, locking mapping normalization, exported ordering, fallback selection, and
provider/store-driven active-binding refresh for `HierarchyControllerProvider` and `SidebarHierarchyController`.
`SidebarHierarchyInteractionController` is covered there as well, so footer action routing and expansion preservation
remain C++ model/controller behavior instead of drifting back into sidebar QML.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/sidebar`` (`docs/src/app/models/sidebar/README.md`)
- 위치: `docs/src/app/models/sidebar`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 최신 책임: footer action dispatch와 hierarchy expansion preservation은
  `SidebarHierarchyInteractionController` C++ 객체가 소유한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
