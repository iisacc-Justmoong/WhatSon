# `src/app/qml/window`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/window`
- Child directories: 1
- Child files: 8

## Child Directories
- `preference`

## Child Files
- `MacNativeMenuBar.qml`
- `IosInlineOnboardingSequence.qml`
- `Onboarding.qml`
- `OnboardingContent.qml`
- `Preference.qml`
- `ProfileControl.qml`
- `QuickNote.qml`
- `TrialStatus.qml`

## Current Notes
- Scene-graph visualization helpers were removed from the runtime window set. This directory now only contains
  user-facing application windows and onboarding/trial surfaces.
- `Onboarding.qml`, `OnboardingContent.qml`, and `TrialStatus.qml` now route visible window geometry through LVRS
  `gap`, `radius`, `stroke`, and `scaleMetric(...)` helpers instead of local pixel literals.
- `IosInlineOnboardingSequence.qml` now keeps the iOS inline onboarding presentation inside the root LVRS workspace
  page, avoiding the `/onboarding` route flip while reusing the shared onboarding content surface.
- Ordinary desktop startup now uses `Onboarding.qml` again as a separate modal shell above `Main.qml`, while Android
  keeps the embedded route-based onboarding and iOS keeps the inline workspace-sequence presentation.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/qml/window`` (`docs/src/app/qml/window/README.md`)
- 위치: `docs/src/app/qml/window`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
