# `src/app/models/sensor`

## Responsibility
Owns read-side hub inspection objects that derive sensor outputs from the unpacked `.wshub` filesystem layout.

## Scope
- Source directory: `src/app/models/sensor`
- Child files:
  - `UnusedResourcesSensor.hpp`
  - `UnusedResourcesSensor.cpp`

## Current Contract
- `UnusedResourcesSensor` scans hub-local `.wsresource` packages and compares them against `<resource ... />`
  embeddings found in note `.wsnbody` documents.
- The sensor returns only packages that are present in the hub but unused by every note.
- The sensor treats RAW note source and package metadata as the source of truth; it does not inspect editor-side DOM
  projections.
