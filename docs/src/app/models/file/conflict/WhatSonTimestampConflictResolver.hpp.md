# `src/app/models/file/conflict/WhatSonTimestampConflictResolver.hpp`

## Responsibility

Declares the timestamp freshness helper used by file conflict checks.

## Contract

- Exposes `isTimestampNewer(...)` for strict read-side freshness checks.

## Boundary

- The resolver does not parse note payloads, mutate headers, or persist files.
- File IO and version capture are outside this module.
