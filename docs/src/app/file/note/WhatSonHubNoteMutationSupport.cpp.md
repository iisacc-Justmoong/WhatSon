# `src/app/file/note/WhatSonHubNoteMutationSupport.cpp`

## Responsibility

This file contains shared helpers for keeping `LibraryNoteRecord` synchronized with note documents
after local mutations.

## Folder Synchronization

The runtime sync path now copies both folder paths and folder UUIDs from the parsed note document
back into the note record. This is important because many higher-level flows perform a write and
then continue using the in-memory record without rebuilding the entire library snapshot.

## Why This Matters

Without this synchronization step, a note header could be updated correctly on disk while the live
note list still held stale path-only folder state until the next full reload.
