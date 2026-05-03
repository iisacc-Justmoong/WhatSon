# `src/app/permissions`

## Responsibility
Hosts application permission integration code:
- platform permission bridges (`ApplePermissionBridge`)
- startup-time permission sequencing (`WhatSonPermissionBootstrapper`)

## Scope
- Source directory: `src/app/permissions`
- Child directories: none
- Child files: 5

## Child Files
- `ApplePermissionBridge.hpp`
- `ApplePermissionBridge.mm`
- `ApplePermissionBridge_stub.cpp`
- `WhatSonPermissionBootstrapper.hpp`
- `WhatSonPermissionBootstrapper.cpp`

## Architectural Notes
- `WhatSonPermissionBootstrapper` was consolidated from `src/app/runtime/permissions` into this domain so permission
  request policy and platform bridge implementations stay in one module boundary.
- Runtime startup orchestrators consume this module rather than owning permission policy directly.

## Dependency Direction
- Consumed by `main.cpp` startup wiring.
- Depends on Qt permission APIs (`QPermission`, `QMicrophonePermission`, `QCalendarPermission`, `QLocationPermission`)
  and Apple bridge request functions.
- iOS simulator-safe exports may define `WHATSON_DISABLE_QT_PERMISSION_REQUESTS=1` from
  `src/app/cmake/runtime/CMakeLists.txt` when `WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY` excludes Qt `permissions`
  plugins. In that mode, the bootstrapper still requests Apple-bridge permissions but skips Qt runtime permission
  steps that would otherwise depend on the excluded plugin type.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/permissions`` (`docs/src/app/permissions/README.md`)
- 위치: `docs/src/app/permissions`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
