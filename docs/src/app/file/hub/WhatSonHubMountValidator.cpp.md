# `src/app/models/file/hub/WhatSonHubMountValidator.cpp`

## Implementation Notes
- The validator owns platform-specific mount/access preflight for Android SAF URIs, restored Android mount paths, and
  iOS security-scoped access restoration.
- After access is secured, the validator checks the minimum hub package contract:
  `.wscontents`, `Library.wslibrary`, `.wsresources`, `*.wsstat`, and the core domain entries under `.wscontents`.
- Startup resolution and onboarding loading now share this one validation path instead of maintaining separate hub
  structure checks.
