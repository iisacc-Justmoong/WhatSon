# `src/app/models/editor/format/ContentsCalloutHtmlRenderer.cpp`

## Responsibility

Implements the shared RAW `<callout>...</callout>` detection and editor RichText HTML projection used by both the
block-normalized renderer and the legacy editor-surface renderer.

## Key Behavior

- Treats callout as a transparent paired tag: the RAW `.wsnbody` source remains `<callout>...</callout>`, while only the
  read-side editor projection receives the callout frame.
- Emits a full-width `#262728` table with Pretendard `12px/12px` white content text.
- Renders the left leading bar as a dedicated `3px` table cell with `bgcolor="#d9d9d9"` plus matching CSS background.
  This avoids relying on Qt RichText's incomplete support for CSS-only `td` backgrounds or `border-radius`, which can
  collapse into a tiny inline mark instead of a vertical bar.
- Leaves layout-only table cells empty, so whitespace-visualization modes cannot expose structural `&nbsp;` markers as
  stray dots or dashes beside the callout body.
- Empty callout bodies render as `&nbsp;` so the frame keeps an occupied editor slot.

## Regression Checks

- The generated callout HTML must not expose literal `<callout>` source tags.
- The leading bar must keep the `width="3"` / `bgcolor="#d9d9d9"` cell contract.
- Unsupported CSS-only shape controls such as `border-radius` must not be required for the visible bar.
