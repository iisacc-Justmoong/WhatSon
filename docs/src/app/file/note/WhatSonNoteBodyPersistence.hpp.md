# `src/app/file/note/WhatSonNoteBodyPersistence.hpp`

## Role
This header defines the shared plain-text persistence contract for note bodies.

It is the boundary between the editor-facing text model and the filesystem-facing `.wsnbody` XML document format.

## Public Contract
- `normalizeBodyPlainText(...)` only normalizes line endings. It must not trim lines or collapse whitespace.
- `plainTextFromBodyDocument(...)` extracts editor text from a `.wsnbody` XML payload while preserving empty paragraphs and whitespace-only paragraphs.
- `firstLineFromBodyPlainText(...)` derives preview text from the first non-empty trimmed line, without mutating the stored plain text.
- `persistBodyPlainText(...)` is the high-level save entry used by hierarchy viewmodels.

## Important Invariants
- Empty `<paragraph></paragraph>` nodes must survive a read/save round-trip when the editor text is unchanged.
- Whitespace-only paragraphs must remain representable in the plain-text editor model.
- The persistence layer must not perform unsolicited body-tag cleanup beyond the serializer that is explicitly chosen for a real body rewrite.
