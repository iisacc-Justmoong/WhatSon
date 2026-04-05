# `src/app/qml/window/TrialStatus.qml`

## Role
`TrialStatus.qml` is the dedicated desktop trial window for the trial packaging tree. It appears on every launch of the trial app and makes the remaining trial time visible without forcing the user to infer state from other runtime behavior.

## Surface
- Root type: `Window`
- Input properties:
  - `hostWindow`
  - `trialActivationPolicy`
- Derived state:
  - authenticated bypass state
  - active or expired trial headline
  - remaining days / elapsed days / install and last-active dates

## Behavior
- The window recenters itself against the host window when one is provided.
- The window stays non-mobile and desktop-oriented: fixed size, decorated, and intended to coexist with the main workspace window.
- All displayed values are read directly from `WhatSonTrialActivationPolicy`, so the view stays read-only and does not mutate trial state.
- Fixed window size, card radius, divider thickness, and headline/body typography now route through `LV.Theme.scaleMetric(...)`
  or `LV.Theme.strokeThin` instead of local pixel literals, so the trial surface follows LVRS density policy.

## Why This Exists
- Trial builds previously had no explicit onboarding/status surface, so users could not tell whether the trial gate was active.
- This window makes the trial lifecycle visible at startup and reduces ambiguity around expiry or authenticated bypass.
