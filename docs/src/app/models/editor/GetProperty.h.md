# `src/app/models/editor/GetProperty.h`

## Responsibility

Declares the `GetProperty` editor-domain attribute capture object.

`GetProperty` is the read-side companion to `SetProperty`. It scans the editable tag at a requested source position,
extracts the tag's attributes, and stores them in the object as in-app key/value state.

## Public Contract

- `readPropertiesFromSource(...)` finds the editable tag at the requested source position and captures its attributes.
- `readPropertiesFromBodyDocument(...)` projects a `.wsnbody` document to editor source and applies the same capture
  path.
- `properties` exposes the current key/value store as a `QVariantMap`.
- `valueKinds` exposes the inferred value kind for each key.
- `tagName` exposes the tag whose attributes are currently captured.
- Quoted attributes are captured as strings. Unquoted `true`/`false`, integer, and floating literals are captured as
  `bool`, integer, and floating values.
- Invalid reads clear the stored properties and set `lastError` so the app does not keep stale property state.

## Signals And Slots

- Signals: `tagNameChanged`, `propertiesChanged`, `lastErrorChanged`, `propertiesCaptured`
- Slots: `clearProperties()`, `clearLastError()`

## Build Contract

- Registered by `src/app/models/editor/CMakeLists.txt`.
- Covered by `test/cpp/suites/editor_get_property_tests.cpp`.
