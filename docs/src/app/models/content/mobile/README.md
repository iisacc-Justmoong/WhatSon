# `src/app/models/content/mobile`

## Responsibility
`src/app/models/content/mobile` contains QObject-based coordinators and state stores used by the mobile workspace flow.

## Scope
- Mirrored source directory: `src/app/models/content/mobile`
- Child directories: 0

## Key Helpers
- `MobileHierarchyRouteStateStore` preserves route-adjacent selection state such as the last observed route path, the
  note-list selection to restore, and pop-repair request ids.
- `MobileHierarchySelectionCoordinator` snapshots sidebar hierarchy bindings and resolves the currently selected
  hierarchy index from active mobile content controllers.
- `MobileHierarchyCanonicalRoutePlanner`, `MobileHierarchyNavigationCoordinator`, and
  `MobileHierarchyBackSwipeCoordinator` keep route transitions deterministic while the mobile scaffold swaps between
  hierarchy, editor, and detail surfaces.
- `MobileHierarchyNavigationCoordinator` now also owns the canonical dismiss mapping for mobile back navigation, so the
  page shell can dismiss `detail -> editor -> note-list -> hierarchy` without depending on a fragile live router-pop
  stack.

## Verification Notes
- `test/cpp/suites/mobile_chrome_tests.cpp` exercises the route-state store normalization path and the sidebar-binding
  snapshot fallback path so mobile navigation helpers stay covered by the in-repo regression gate.
- The same suite now also pins `dismissPagePlan(...)` plus the source-locked `dismissCurrentPage()` wiring in
  `MobileHierarchyPage.qml`, so mobile back navigation cannot regress to raw `router.back()` pops from the editor.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/content/mobile`` (`docs/src/app/models/content/mobile/README.md`)
- 위치: `docs/src/app/models/content/mobile`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
