# `src/app/platform/Apple/AppleSecurityScopedResourceAccess.mm`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/platform/Apple/AppleSecurityScopedResourceAccess.mm`
- Source kind: Objective-C++ implementation
- File name: `AppleSecurityScopedResourceAccess.mm`
- Approximate line count: 396

## Extracted Symbols
- Declared namespaces present: yes
- QObject macro present: no

### Classes and Structs
- `ScopedResourceRegistry`

### Enums
- None detected during scaffold generation.

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
- Provider-backed iOS URLs are no longer rejected up front just because `QUrl::isLocalFile()` is false; the
  implementation now resolves a filesystem path from the underlying `NSURL`, supports parent-directory derivation via
  `URLByDeletingLastPathComponent`, and reuses that path for bookmark persistence/start-access bookkeeping.
- The implementation now also supports stripping multiple ancestor components from a picked provider URL, so a file
  chosen from inside a `.wshub` package can be remapped to the package root URL before bookmark persistence and access
  restoration.
