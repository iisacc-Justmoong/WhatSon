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
