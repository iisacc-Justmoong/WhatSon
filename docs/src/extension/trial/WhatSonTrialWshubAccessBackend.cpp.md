# `src/extension/trial/WhatSonTrialWshubAccessBackend.cpp`

## Role
Implements the `.wshub` access gate for the optional trial extension.

## Behavior
- Local file-system paths are normalized with `QDir::cleanPath(...)`.
- URI targets keep their original text form so Android document URIs are preserved.
- The backend recognizes both local `.wshub` directories and URI paths that end with `.wshub`.
- If `WhatSonRegisterManager::authenticated()` is `true`, the backend returns an allowed decision before any trial register checks run.
- When the trial is expired, the backend denies access and reports the last active date in ISO format.
- Local `.wshub` paths read `.whatson/trial_register.xml` and compare its `key` value against the persisted in-app trial key.
- Local `.wshub` paths are also denied when `.whatson/trial_register.xml` is missing, because the key comparison cannot be completed.
- A missing or malformed local register file key denies access when the file is present, while document URIs remain expiry-only because the XML payload is not locally readable.
- The register `deviceUUID` is loaded for completeness, but it does not affect the access decision.

## Integration Intent
- The module lives under `src/extension/trial` so downstream startup/load flows can opt in without turning the extension into a mandatory core dependency.
