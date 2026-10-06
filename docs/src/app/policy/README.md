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

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/policy``  ( `docs/src/app/policy/README.md` )
- Location:  `docs/src/app/policy`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
