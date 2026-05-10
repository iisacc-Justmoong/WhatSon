# `src/app/models/editor/SetProperty.cpp`

## Responsibility

Implements dynamic attribute mutation for editor-facing `.wsnbody` source tags.

## Current Rules

- The cursor must point inside an opening tag such as `<resource />` or `<header>`.
- Closing tags, XML declarations, and comment-like tags are rejected.
- Existing attributes with the same name are replaced in place.
- Missing attributes are inserted before the self-closing slash or before the opening tag's closing `>`.
- String values are quoted and XML-escaped. Numeric and boolean values are written as unquoted RAW literals because
  the editor command already supplies strongly typed `QVariant` values.
- `.wsnbody` document mutation delegates all projection and serialization work to `WhatSon::NoteBodyPersistence`.

## Regression Notes

- `test/cpp/suites/editor_set_property_tests.cpp` verifies dynamic property names, inferred string/int/float/bool
  value serialization, update behavior, invalid input rejection, and body-document round-tripping.
