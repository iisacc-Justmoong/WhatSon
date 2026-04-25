# `src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp`

## Responsibility

`WhatSonRuntimeParallelLoader.cpp` builds LVRS `BootstrapParallelTask` entries for requested domain snapshot work,
runs them through `lvrs::runBootstrapParallelTasks(...)`, and then applies the immutable snapshot payloads back onto
the main-thread viewmodels after the requested domain set succeeds.

## Requested Domain Selection

The loader now accepts an explicit requested-domain mask. Startup can therefore load only the
critical first-frame domains while leaving only low-priority hierarchies for deferred follow-up
work.

## Shared Library Snapshot Rule

When both the library and bookmarks domains are present, the loader now indexes the library domain
once and derives bookmarks from that shared library snapshot.

This removes the previous duplicate `.wshub` traversal where the bookmarks task reparsed the same
library note set independently.

## Failure Behavior

- If the library snapshot fails, the derived bookmarks result fails with the same error.
- If the library domain is absent, the loader falls back to the standalone bookmarks snapshot path.
- The loader now stages every requested domain, including `hub.runtime`, through LVRS `BootstrapParallel`, and only
  applies snapshots back into live viewmodels/runtime stores when every requested domain succeeded.
- If any requested domain fails, the loader returns failure without partially mutating the current
  runtime state.

## Test Coverage

`test/cpp/suites/runtime_parallel_loader_tests.cpp` keeps this loader on LVRS `BootstrapParallel`, prevents direct
`QThread`/`QEventLoop` worker management from returning, and preserves the all-or-nothing apply gate.
