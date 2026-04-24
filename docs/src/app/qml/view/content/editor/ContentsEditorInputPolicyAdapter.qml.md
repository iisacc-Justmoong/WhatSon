# `src/app/qml/view/content/editor/ContentsEditorInputPolicyAdapter.qml`

## Responsibility

`ContentsEditorInputPolicyAdapter.qml` centralizes editor input policy decisions that should eventually move into LVRS.
It is not a visual component; it is the app-local adapter for native text-input priority, tag-command shortcut gating,
context-menu trigger gating, focused programmatic text sync, and post-mutation focus restoration.

## Current Contract

- OS/Qt `TextEdit` remains the authority for ordinary note editing, IME composition, preedit text, repeated
  Backspace/Delete, selection gestures, cursor movement, and platform keyboard behavior.
- App-level shortcuts are enabled only when the native text-input session does not own the keyboard.
- Tag-management shortcuts are a subset of that shortcut surface and are the only editor commands allowed outside the
  native text-input path.
- Mobile long-press context-menu handling is disabled while the native text-input session is active, so iOS text
  selection and caret gestures stay with the platform.
- Programmatic host text sync is deferred while composition is active or while a focused native-input editor has a
  local text edit that has not blurred yet.
- Plain text-edit source mutations mark their focus request with `reason: "text-edit"`; the adapter blocks those
  same-block focus restores during a native-priority focused input session.

## LVRS Promotion Target

This adapter exists as the app-side staging point for a future LVRS primitive. LVRS should own the common rules for
native text-input sessions, command-surface gating, and pointer context-menu symmetry so app QML no longer repeats those
decisions per editor surface.
