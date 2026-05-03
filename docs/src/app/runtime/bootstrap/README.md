# `src/app/runtime/bootstrap`

## Role
This directory contains bootstrap-time composition helpers extracted from `main.cpp`.

The module keeps `main.cpp` focused on orchestration flow while moving repetitive wiring blocks into
dedicated single-purpose helpers.

## Files
- `WhatSonAppLaunchSupport`: parses launcher flags and classifies whether startup can enter the workspace immediately.
- `WhatSonQmlLaunchSupport`: routes QML root creation and window activation through LVRS app-entry helpers, preserving
  `QmlRootLoadResult` for lifecycle and foreground-service bootstrap.
- `WhatSonQmlInternalTypeRegistrar`: registers QML-instantiated internal bridge types through an LVRS manifest.
- `WhatSonHubSyncWiring`: wires local mutation signals into `WhatSonHubSyncController`.
- `WhatSonQmlContextBinder`: applies the workspace LVRS context/Controller bind plan before root QML load.

`main.cpp` retains foreground-service startup ownership because scheduler and permission requests are composition-root
policy. That startup is guarded by LVRS `ForegroundServiceGate` after `WhatSonQmlLaunchSupport` has produced and
activated a visible workspace root.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/runtime/bootstrap`` (`docs/src/app/runtime/bootstrap/README.md`)
- 위치: `docs/src/app/runtime/bootstrap`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
- 런타임 기준: `WhatSonQmlLaunchSupport`는 LVRS `QmlRootLoadResult`를 보존하고 lifecycle/foreground-service
  부트스트랩에 같은 root/window 결과를 전달한다.
