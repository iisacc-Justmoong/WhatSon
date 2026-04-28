# `src/app/models/editor/input/ContentsEditorInputPolicyAdapter.qml`

## Responsibility

`ContentsEditorInputPolicyAdapter.qml` centralizes editor input policy decisions that should eventually move into LVRS.
It is not a visual component; it is the app-local adapter for native text-input priority, tag-command shortcut gating,
context-menu trigger gating, focused programmatic text sync, and post-mutation focus restoration.

## Current Contract

- OS/Qt `TextEdit` remains the authority for ordinary note editing, IME composition, preedit text, repeated
  Backspace/Delete, selection gestures, cursor movement, and platform keyboard behavior.
- Ordinary app-level shortcuts are enabled only when the native text-input session does not own the keyboard. Any
  focused note-body `TextEdit` counts as a native text-input session on desktop and mobile, so platform text-navigation
  and selection chords are not preempted by ordinary shortcuts.
- The adapter no longer accepts a separate `command surface enabled` readiness flag.
  Shortcut and context-menu gating derive directly from parse-mounted note-body state plus active structured-editor
  mode, so `.wsnbody` remains the only readiness authority.
- Tag-management shortcuts are deliberately separate from the ordinary shortcut surface. Inline style wrapping and
  structured tag insertion remain available while an editor is focused, but they still stand down during native
  composition/preedit.
- Focused text delegates may forward only those explicit tag-management key chords to the host, so formatting shortcuts
  can mutate RAW `<bold>`/`<italic>`/`<highlight>` tags without reintroducing generic text-input key handling.
- Mobile long-press context-menu handling is disabled while the native text-input session is active, so iOS text
  selection and caret gestures stay with the platform.
- Programmatic host text sync is deferred while composition is active or while a focused native-input editor has a
  local text edit or cursor/selection gesture in flight that has not blurred yet.
  This keeps iOS keyboard trackpad/selection gestures from being reset by host-side projection resync before the user
  has typed any new text.
- Plain text-edit source mutations mark their focus request with `reason: "text-edit"`; the adapter blocks those
  same-block focus restores during a native-priority focused input session.

## LVRS Promotion Target

This adapter exists as the app-side staging point for a future LVRS primitive. LVRS should own the common rules for
native text-input sessions, command-surface gating, and pointer context-menu symmetry so app QML no longer repeats those
decisions per editor surface.
