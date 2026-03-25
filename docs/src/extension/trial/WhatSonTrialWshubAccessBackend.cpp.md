# `src/extension/trial/WhatSonTrialWshubAccessBackend.cpp`

## Role
Implements the `.wshub` access gate for the optional trial extension.

## Behavior
- Local file-system paths are normalized with `QDir::cleanPath(...)`.
- URI targets keep their original text form so Android document URIs are preserved.
- The backend recognizes both local `.wshub` directories and URI paths that end with `.wshub`.
- When the trial is expired, the backend denies access and reports the last active date in ISO format.
- Future key-matching checks for hub access should read the trial-only registration payload from `.whatson/trial_register.xml`.

## Integration Intent
- The module lives under `src/extension/trial` so downstream startup/load flows can opt in without turning the extension into a mandatory core dependency.
