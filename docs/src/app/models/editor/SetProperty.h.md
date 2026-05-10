# `src/app/models/editor/SetProperty.h`

## Responsibility

Declares the `SetProperty` editor-domain tag-attribute mutation object.

`SetProperty` is intentionally separate from `SetTag`: tag names may be fixed by the calling editor command, but
attribute names are runtime string inputs.

## Public Contract

- `setPropertyInSource(...)` finds the editable tag at the requested source position and inserts or updates one
  attribute.
- `setPropertyInBodyDocument(...)` projects a `.wsnbody` document to editor source, applies the same source mutation,
  and serializes the body document back through `WhatSon::NoteBodyPersistence::serializeBodyDocument(...)`.
- Property names are validated as XML-like attribute names, but are not restricted to a static allow-list.
- Value type is inferred from `QVariant`:
  - `QString` / `QByteArray` / `QChar` -> quoted XML-escaped string
  - signed and unsigned integer variants -> unquoted integer literal
  - `float` / `double` -> unquoted C-locale floating literal
  - `bool` -> unquoted `true` or `false`
- Unsupported value types and invalid property names return an invalid result and leave the source/document unchanged.

## Signals And Slots

- Signals: `lastErrorChanged`, `propertySet`
- Slots: `clearLastError()`

## Build Contract

- Registered by `src/app/models/editor/CMakeLists.txt`.
- Covered by `test/cpp/suites/editor_set_property_tests.cpp`.
