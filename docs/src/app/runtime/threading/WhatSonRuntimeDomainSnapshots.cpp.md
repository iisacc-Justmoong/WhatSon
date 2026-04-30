# `src/app/runtime/threading/WhatSonRuntimeDomainSnapshots.cpp`

## Responsibility

This file builds startup-time runtime snapshots for the main thread. It loads note records, smart
bucket projections, and persisted hierarchy files in a worker-thread friendly form.

`WhatSonLibraryIndexedState` is now the shared backend for the library-side note projections. The
snapshot loader uses that backend to materialize `all`, `draft`, and `today` once, then exposes a
`buildBookmarks(...)` helper that derives the bookmarks domain from the already indexed library note
set instead of reparsing the mounted hub.

## Folder Snapshot Role

For library folders, the snapshot loader parses `Folders.wsfolders` into
`WhatSonFolderDepthEntry` rows and exposes both the file path and the normalized folder entries to
the startup pipeline.

## UUID Migration Behavior

The snapshot loader now forwards the parser's `outUuidMigrationRequired` signal and rewrites
`Folders.wsfolders` immediately when a legacy file had no persisted UUIDs.

That keeps startup behavior aligned with direct `LibraryHierarchyController::loadFromWshub()` loads:

- a legacy folder tree receives stable UUIDs once,
- the upgraded file is persisted,
- the next launch reuses the same folder identities instead of generating a different in-memory set.

## Failure Policy

If the snapshot loader detects that UUID migration is required but cannot rewrite the folder file, it
marks the snapshot as failed and propagates the write error. This avoids silently continuing with a
session-local identity map.

Hub runtime loading now follows the same staging rule. `loadHubRuntime(...)` materializes a
temporary `WhatSonHubRuntimeStore` inside `HubRuntimeSnapshot` and hands that staged copy back to
`WhatSonRuntimeParallelLoader`. The live runtime store is updated only after every requested domain
finished successfully.

## Resource Snapshot Fallback

리소스 스냅샷 로더는 먼저 `.wscontents/Resources.wsresources`를 읽는다.
그 결과가 비어 있으면 허브의 모든 리소스 루트(`.wsresources` + `*.wsresources`)를 직접 스캔해서 flat
`.wsresource` 패키지 목록을 `snapshot.values`로 채운다.

즉 startup path 역시 raw asset file이 아니라 `.wsresource` package path를 기준으로 움직인다.
