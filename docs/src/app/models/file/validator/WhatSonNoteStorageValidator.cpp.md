# `src/app/models/file/validator/WhatSonNoteStorageValidator.cpp`

## Behavior Summary
The implementation provides path resolution helpers and an in-place normalizer for `.wsnote` directories.

## Normalization Flow
`normalizeWsnotePackage(...)` executes the following sequence:

1. Resolve a materialized note directory from the record.
2. Skip non-materialized or non-`.wsnote` paths.
3. Derive canonical target names from the note directory stem:
   - `<stem>.wsnhead`
   - `<stem>.wsnbody`
   - `<stem>.wsnversion`
   - `<stem>.wsnpaint`
4. For each required suffix:
   - Keep existing canonical file if present.
   - Otherwise migrate the first legacy file with the same suffix into the canonical target.
   - If no source exists, write an empty default document for that suffix.
5. Remove all non-allowed files.
6. Remove all subdirectories recursively (including hidden directories).

## Default Document Policy
- Missing `.wsnhead` is synthesized as a minimal valid header XML.
- Missing `.wsnbody` is synthesized as a minimal body XML with an empty paragraph.
- Missing `.wsnversion` is synthesized as `whatson.note.version.store` JSON.
- Missing `.wsnpaint` is synthesized as `WHATSONNOTEPAINT` XML.

## Pollution Removal Scope
- Files such as `*.wsnlink`, `*.wsnhistory`, and any unknown sidecar are deleted.
- Hidden directories (for example `.meta`) are explicitly included in cleanup using `QDir::Hidden`.
- Legacy directories (for example `attachments`) are removed recursively.

## Error Handling
- Every filesystem mutation uses `WhatSonSystemIoGateway`.
- Any failed read/write/delete operation returns `false` and propagates a descriptive message through `errorMessage`.
- Successful normalization is idempotent for already-clean packages.
