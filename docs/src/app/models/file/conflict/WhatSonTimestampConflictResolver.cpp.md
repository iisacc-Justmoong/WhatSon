# `src/app/models/file/conflict/WhatSonTimestampConflictResolver.cpp`

## Runtime Behavior

- Parses the local note timestamp format `yyyy-MM-dd-hh-mm-ss` and Qt ISO timestamp variants.
- `mergeBodyByTimestamp(...)` treats filesystem advancement after the base pull timestamp as the conflict trigger.
- On conflict, it compares filesystem and incoming timestamps and returns the newer body's source text.
- Equal timestamps keep the incoming body to preserve the active editor save as the tie-breaker.

## Tests

- `test/cpp/suites/timestamp_conflict_resolver_tests.cpp` covers resolver-only winner selection and file-store
  integration for both filesystem-newer and incoming-newer cases.
