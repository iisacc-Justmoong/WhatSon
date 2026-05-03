# `src/app/policy`

## Role
This directory contains explicit architectural constraints for the application layer graph.

The policy module is intentionally small. Its job is not to model business behavior. Its job is to define and enforce dependency direction during runtime wiring so that view, controller, store, parser, creator, and filesystem concerns do not silently collapse into each other.

## Files
- `ArchitecturePolicyLock.hpp`: public layer vocabulary and policy helper declarations.
- `ArchitecturePolicyLock.cpp`: dependency matrix, lock state, and runtime verification logging.

## Operational Model
- During startup, mutable dependency injection is allowed.
- Once root wiring is complete, the composition root locks the policy.
- After the lock, late reassignment of critical runtime collaborators should be rejected.
- Runtime bridge code can also call dependency verification helpers to log illegal layer edges in production code paths.
- Major setter/wiring seams now share explicit helpers for both cases: `verifyMutableWiringAllowed(...)` blocks post-lock rewiring, and `verifyMutableDependencyAllowed(...)` combines the lock rule with role-based layer verification such as `View -> Controller` and `Controller -> Store`.

## Why This Directory Matters
This module is small, but it acts as the repository's explicit statement of intended architecture. When documentation and runtime behavior drift apart, this is the first place that should be corrected.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/policy`` (`docs/src/app/policy/README.md`)
- 위치: `docs/src/app/policy`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
