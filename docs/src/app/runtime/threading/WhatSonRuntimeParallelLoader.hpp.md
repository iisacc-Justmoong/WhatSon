# `src/app/runtime/threading/WhatSonRuntimeParallelLoader.hpp`

## Role
`WhatSonRuntimeParallelLoader` is the concrete parallel loader used for `.wshub` runtime bootstrap.

## Interface Alignment
- Implements `IWhatSonRuntimeParallelLoader`.
- Re-exports the shared nested types through `using` aliases so existing implementation code stays stable.
