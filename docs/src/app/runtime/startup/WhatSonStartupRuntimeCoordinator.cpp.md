# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp`

## Resources Reload Path

`reloadResourcesDomainIntoRuntime(...)`는 `RequestedDomains`를 다음처럼 좁혀서 실행한다.

- `resources = true`
- `hubRuntimeStore = true`
- 나머지 domain = `false`

이렇게 하면 현재 허브의 resource package 목록과 hub runtime domain 값은 갱신되지만, startup 단계에서 아직 deferred 상태인 `event`/`preset` hierarchy까지 강제로 끌어올리지는 않는다.
