# `src/app/models/file/hub/WhatSonHubCreator.hpp`

## Role
Declares the filesystem-facing factory that writes a new hub scaffold into a pre-materialized `.wshub` package root.

## Public API
- `createHub(...)`: creates a sanitized hub package below the configured workspace hubs root.
- `createHubAtPath(...)`: creates a hub package at an explicit destination path and appends `.wshub` when missing.
- `requiredRelativePaths(...)`: returns the directory scaffold that must exist before file payloads are written.

## Creation Contract
- A successful call delegates package-root materialization to `WhatSonHubPackager`, then writes the base scaffold, including `.whatson/hub.json`.
- Existing hub directories are treated as hard failures and are never overwritten in place.
