# `src/extension`

## Role
This directory is reserved for optional product extensions that should not automatically become mandatory app or daemon build inputs.

## Directories
- `appintent`: reserved space for platform intent integrations.
- `trial`: local trial-entitlement policy kit for evaluation builds.
- `widget`: reserved space for optional widget packaging code.

## Build Policy
- Extension code is opt-in by design.
- New modules here should avoid silently expanding the mandatory root build graph.
- Validation for this directory should rely on runtime integration and targeted diagnostics.
