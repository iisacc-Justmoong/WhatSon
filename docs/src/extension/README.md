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

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/extension``  ( `docs/src/extension/README.md` )
- Location:  `docs/src/extension`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
