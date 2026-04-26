# `src/app/models/editor/input/ContentsCalloutBlockController.qml`

## Responsibility

Owns non-visual callout block editing state for `ContentsCalloutBlock.qml`.

It tracks focused live text, computes cursor geometry, applies focus requests, clears stale selection, and emits
committed callout text changes to the structured flow.

## Contract

- The callout view remains responsible for layout and visual styling.
- The nested inline editor stays in native `TextEdit` input mode.
- Committed text changes are emitted with the previous live text snapshot so the host can reject stale source
  mutations.

## Boundary

Callout typing logic belongs here, but generic key interception does not. Native editing keys stay with the nested
inline editor.
