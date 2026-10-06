# `src/app/models/content`

## Responsibility
`src/app/models/content` owns non-editor content-surface helper objects that sit between app state and LVRS/QML hosts.

## Scope
- Mirrored source directory: `src/app/models/content`
- Child directories: 1
- Child files: 0

## Child Directories

## Architectural Notes
  keep compact workspace navigation deterministic across note/detail/editor transitions.
- Editor-domain model objects now live in the explicit `src/app/models/editor` shard; content helpers must not depend on
  parser, projection, renderer, session, minimap, or structured-document editor objects.

## Verification Notes
  sidebar-binding resolution regression checks.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/content``  ( `docs/src/app/models/content/README.md` )
- Location:  `docs/src/app/models/content`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
