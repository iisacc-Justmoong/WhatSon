# `src/extension/trial/WhatSonTrialRegisterXml.cpp`

## Role
Implements `.whatson/trial_register.xml` serialization for the optional trial extension.

## Behavior
- Writes a small XML payload with `<deviceUUID>`, `<key>`, and `<signature>` elements.
- Normalizes the input identity before persisting it.
- Computes an HMAC-SHA256 signature over the canonical payload using the local register-integrity secret.
- Uses `QSaveFile` so partially written register files are not committed over valid data.
- Rejects malformed, incomplete, or integrity-mismatched register files when loading them back.
