# `src/app/policy/ArchitecturePolicyLock.hpp`

## Role
This header defines the public vocabulary for application-layer dependency control.

It exposes two separate concepts.
- A static dependency policy expressed through the `Layer` enum and helper functions.
- A runtime lock expressed through `ArchitecturePolicyLock`, used to freeze startup-time wiring once the root object graph is complete.

## Public API
- `enum class Layer`: canonical layer names used throughout runtime dependency verification.
- `layerName(Layer)`: string form used in logs and diagnostics.
- `isDependencyAllowed(Layer from, Layer to)`: pure policy query without side effects.
- `assertDependencyAllowed(...)`: returns a boolean and optionally formats an error message.
- `verifyDependencyAllowed(...)`: same rule check, but intended for production call sites that should emit warnings when a bad edge is attempted.
- `verifyMutableWiringAllowed(...)`: rejects post-lock runtime rewiring attempts and can format a caller-facing reason.
- `verifyMutableDependencyAllowed(...)`: applies both the post-lock mutation rule and the layer dependency rule in one shared helper.
- `ArchitecturePolicyLock::isLocked()` and `ArchitecturePolicyLock::lock()`: global startup freeze.
- `ArchitecturePolicyLock::unlockForTests()`: test-only escape hatch used to isolate process-global lock state between regression cases.

## Usage Pattern
Typical consumers are startup-time composition code and bridge-like objects that connect QML-facing views to QObject-based viewmodels or stores.

The expected sequence is this.
1. Build the object graph.
2. Inject required collaborators.
3. Call `ArchitecturePolicyLock::lock()`.
4. Reject later structural rewiring attempts.

## Important Constraint
This header only defines a small policy language. It does not automatically rewrite architecture by itself. Production code must call the verification helpers if runtime enforcement is desired.

The current runtime contract is role-based rather than folder-based: bridge/QML-facing setter paths are treated as `View -> ViewModel`, and ViewModel store attachment paths are treated as `ViewModel -> Store`.
