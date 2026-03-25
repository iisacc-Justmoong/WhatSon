# `src/extension/trial/WhatSonTrialWshubAccessBackend.hpp`

## Role
Declares the optional backend that blocks `.wshub` access after the local 90-day trial expires.

## Public API
- `evaluateAccess(...)`: returns a structured decision for a local `.wshub` path or a document URI that points to a `.wshub` package.
- `canAccess(...)`: convenience boolean wrapper that also exposes a denial message.

## Decision Model
- Non-`.wshub` targets are ignored and remain allowed.
- `.wshub` targets are allowed only while `WhatSonTrialActivationPolicy` reports an active trial state.
- Expired decisions include the install-derived trial state and a user-facing denial string.
- Trial-only hub identity checks are expected to use `.whatson/trial_register.xml`, not the licensed-build register filename.
