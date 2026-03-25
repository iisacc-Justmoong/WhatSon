# `src/extension/trial/WhatSonTrialRegisterXml.hpp`

## Role
Declares the trial-only XML reader and writer for local hub registration files.

## Public API
- `registerFileName()`: returns the fixed trial register filename `trial_register.xml`.
- `registerFilePath(...)`: resolves `.whatson/trial_register.xml` under a local `.wshub` root.
- `exists(...)`: reports whether the local hub already has a trial register file.
- `loadRegister(...)`: reads the normalized `deviceUUID` and `key` pair from the XML file.
- `writeRegister(...)`: stores a normalized client identity into the XML file.

## Scope
- The helper only accepts local hub paths.
- Product register filenames remain out of scope for this trial-only module.
- Trial creation code is expected to feed `writeRegister(...)` with the persisted identity from `WhatSonTrialClientIdentityStore::ensureIdentity()`.
