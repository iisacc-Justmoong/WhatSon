# `src/app/runtime/threading/IWhatSonRuntimeParallelLoader.hpp`

## Role
`IWhatSonRuntimeParallelLoader` defines the runtime-domain loading contract used by startup coordination.

## Contract
- Shared `Targets`, `RequestedDomains`, and `DomainLoadResult` structs.
- `loadFromWshub(...)` for domain snapshot application.

## Notes
- Startup coordination now depends on this loader interface and receives the concrete loader via injection from `main.cpp`.
