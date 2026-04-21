# `src/app/models/file/hub/WhatSonHubMountValidator.hpp`

## Role
`WhatSonHubMountValidator` resolves a persisted or user-selected hub path into a mounted local `.wshub` package and
reports whether that package is structurally complete enough to enter runtime bootstrap.

## Interface Alignment
- `resolveMountedHub(...)` accepts a raw path plus an optional access bookmark.
- The return payload distinguishes successful mounts from invalid/incomplete hub packages without throwing.
