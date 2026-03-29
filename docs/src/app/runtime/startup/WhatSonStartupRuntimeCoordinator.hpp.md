# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp`

## Role

startup runtime coordinator는 `.wshub` 로드 요청을 어떤 runtime domain 집합으로 실행할지 결정하는 조정자다.

## Public Entry Points

- `loadHubIntoRuntime(...)`
  전체 runtime domain을 다시 적재한다.
- `loadStartupHubIntoRuntime(...)`
  startup first-frame 경로에 맞춰 일부 hierarchy를 deferred bootstrap으로 남긴다.
- `reloadResourcesDomainIntoRuntime(...)`
  resource import 같은 국소 변경 뒤 `hub.runtime + resources`만 다시 적재한다.

## Why The Resources-Only Reload Exists

ApplicationWindow 파일 드롭 import는 리소스 저장소와 `Resources.wsresources`만 바꾸므로, 매번 전체 runtime을 다시 적재하면 startup deferred bootstrap 정책까지 불필요하게 끝내 버릴 수 있다.
이 entrypoint는 그 부작용 없이 리소스 영역만 반영하기 위한 경로다.
