# `src/app/models/file/note/WhatSonHubNoteMutationSupport.cpp`

## Responsibility

This file contains shared helpers for keeping `LibraryNoteRecord` synchronized with note documents
after local mutations.

It no longer owns note sidecar manifest generation. Package scaffold responsibility now lives in
`WhatSonLocalNoteFileStore`.

The former shared IO gateway has also been removed. This implementation now performs the small filesystem operations
owned by note mutation flows directly through Qt primitives: `QDir` for directories, `QFile` for reads/removals, and
`QSaveFile` for atomic UTF-8 overwrites.

## Folder Synchronization

The runtime sync path now copies both folder paths and folder UUIDs from the parsed note document
back into the note record. This is important because many higher-level flows perform a write and
then continue using the in-memory record without rebuilding the entire library snapshot.

## Why This Matters

Without this synchronization step, a note header could be updated correctly on disk while the live
note list still held stale path-only folder state until the next full reload.
