# `src/extension/trial/WhatSonTrialRegisterXml.cpp`

## Role
Implements `.whatson/trial_register.xml` serialization for the optional trial extension.

## Behavior
- Writes a small XML payload with `<deviceUUID>` and `<key>` elements.
- Normalizes the input identity before persisting it.
- Uses `QSaveFile` so partially written register files are not committed over valid data.
- Rejects malformed or incomplete register files when loading them back.
