# `src/app/models/file/diff/WhatSonNoteVersionStateCodec.hpp`

## Role
Declares the codec boundary for `.wsnversion` state text.

## Contract
- Creates serialized empty version-state text for a note ID.
- Serializes `WhatSonNoteVersionState` into JSON text.
- Parses JSON text into `WhatSonNoteVersionState`.

## Boundary
- The codec owns schema fields and compatibility defaults.
- It does not decide when to capture, checkout, rollback, or prune snapshots.
