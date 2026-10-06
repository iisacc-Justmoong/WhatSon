# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp`

<a id="responsibility"></a>

## 책임
`WhatSon.App.Internal` 아래에 QML 내부 브리지 유형을 등록하는 부트스트랩 도우미를 선언합니다.

<a id="contract"></a>

## 계약
- `internalQmlTypeRegistrationManifest()` : 남은 쉘 수준 `WhatSon.App.Internal` 유형 등록을 LVRS 매니페스트로 선언합니다.
- `registerInternalQmlTypes()`: `main.cpp`가 LVRS 베이스라인 타입 등록 후 호출한 원샷 등록 엔트리포인트이며, LVRS 등록 보고서를 반환합니다.

<a id="rationale"></a>

## 이론적 근거
QML 유형 등록은 런타임 도메인 동작이 아닌 구성 인프라입니다. `main.cpp` 외부에 유지하면 컴포지션 루트 표면이 줄어들고, LVRS 매니페스트 결과는 내부 브리지 유형을 등록할 수 없는 경우 구체적인 진단으로 시작이 실패하도록 합니다.
