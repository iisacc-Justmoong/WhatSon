# `src/app/models/editor/GetProperty.cpp`

## Responsibility

Implements in-app attribute capture for editor-facing `.wsnbody` source tags.

## Current Rules

- The cursor must point inside an opening tag such as `<resource property1="asset" />`.
- Closing tags, XML declarations, and comment-like tags are rejected.
- Attribute names are parsed as XML-like names and stored as dynamic map keys.
- Quoted values are XML-unescaped strings.
- Unquoted values are inferred in this order: boolean, integer, floating point, then string fallback.
- Repeated attribute names keep the last parsed value in the public `properties` map, while the result's `attributes`
  list preserves the parsed sequence for callers that need it.
- `.wsnbody` document capture delegates projection work to `WhatSon::NoteBodyPersistence`.

## Regression Notes

- `test/cpp/suites/editor_get_property_tests.cpp` verifies typed key/value capture, object state updates, stale-state
  clearing on invalid reads, and body-document projection.
