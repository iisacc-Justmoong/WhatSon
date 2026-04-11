# Structured Editor Regression Checklist

This repository does not operate automated or scripted tests. Use this checklist as manual regression coverage for
structured document-flow editor changes.

## Agenda Typing
- Typing continuously inside an existing agenda task must keep the caret in that same task instead of snapping to the
  task start.
- Pressing `Enter` inside an agenda task must still create the next task or exit the agenda according to the existing
  backend rule.

## Callout Typing
- Typing continuously inside a callout must keep the caret at the edited position after each RAW rewrite/reparse.
- Pressing `Enter` twice on a trailing empty callout line must still exit the callout block.

## Structured Shortcuts
- In structured-flow mode, `Cmd+Opt+T`, `Cmd+Opt+C`, and `Cmd+Shift+H` invoked from the middle of a text block must
  insert at the live caret position, not only after the whole block.
- The same shortcuts invoked while focus is inside an agenda or callout must keep wrappers standalone instead of nesting
  a new proprietary block inside existing task/callout body content.
