# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp`

## Role

The startup runtime coordinator decides which runtime domain set should execute for each `.wshub` load request.

## Public Entry Points

- `loadHubIntoRuntime(...)`
  Reloads the full runtime domain set.
- `loadStartupHubIntoRuntime(...)`
  Preserves deferred hierarchy bootstrap rules for the startup first-frame path.
- `reloadResourcesDomainIntoRuntime(...)`
  Reloads only `hub.runtime + resources` after a localized resource import change.

## Why The Resources-Only Reload Exists

Resource import entrypoints, including the macOS menu-based file picker, only mutate the resource store and
`Resources.wsresources`. Reloading the entire runtime after every import would also force unrelated deferred-startup
domains to complete early. This entrypoint exists to reflect the resource change without paying that broader cost.
