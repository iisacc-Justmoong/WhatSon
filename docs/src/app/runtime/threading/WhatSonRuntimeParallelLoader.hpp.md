# `src/app/runtime/threading/WhatSonRuntimeParallelLoader.hpp`

<a id="role"></a>

## 역할
`WhatSonRuntimeParallelLoader`는 `.wshub` 런타임 부트스트랩에 사용되는 콘크리트 LVRS `BootstrapParallel` 지원 로더입니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- `IWhatSonRuntimeParallelLoader`를 구현합니다.
- 기존 구현 코드가 안정적으로 유지되도록 `using` 별칭을 통해 공유 중첩 유형을 다시 내보냅니다.
