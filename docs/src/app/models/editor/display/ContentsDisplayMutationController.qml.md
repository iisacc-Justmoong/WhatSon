# `src/app/models/editor/display/ContentsDisplayMutationController.qml`

## Responsibility

Owns QML-runtime mutation orchestration for the editor display host.

## Boundary

- Applies RAW `.wsnbody` mutation plans through editor-domain coordinators.
- Delegates public access through `ContentsDisplayMutationViewModel`.
- Must preserve RAW source as the only write authority.
