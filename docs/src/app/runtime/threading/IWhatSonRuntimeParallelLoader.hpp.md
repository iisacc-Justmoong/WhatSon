# `src/app/runtime/threading/IWhatSonRuntimeParallelLoader.hpp`

<a id="role"></a>

## 역할
`IWhatSonRuntimeParallelLoader`는 시작 조정에 사용되는 런타임 도메인 로딩 계약을 정의합니다.

<a id="contract"></a>

## 계약
- `Targets`, `RequestedDomains` 및 `DomainLoadResult` 구조체를 공유했습니다.
- 도메인 스냅샷 애플리케이션용 `loadFromWshub(...)`.

<a id="notes"></a>

## 메모
- 이제 시작 조정은 이 로더 인터페이스에 따라 달라지며 `main.cpp`에서 주입을 통해 콘크리트 로더를 받습니다.
- `RequestedDomains{}`는 일반 런타임 로딩에 대한 전체 로드 요청으로 남아 있습니다. 지속적인 시작은 첫 번째 워크스페이스 유휴 전환 전에 도메인 요청을 더 이상 생성하지 않으므로, 이러한 기본값은 초기 페인트 경로에 더 이상 적용되지 않습니다.
