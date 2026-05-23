# `src/app/models/editor/component/style.cpp`

## Responsibility

Implements the editor-domain projection and recovery helpers for the proprietary `.wsnbody` `<style ...>` wrapper.

## Current Rules

- LVRS text tokens such as `Title`, `Title2`, `Subtitle`, `Header`, `Header2`, `Body`, `Description`, `Caption`,
  `Footnote`, and legacy `Disabled` map to the same Pretendard size, weight, style name, and line-height recipe used by
  `ContentEditorToolbar.qml` style-menu preview descriptors. Token-derived style projection intentionally does not add
  per-style colors; styled spans inherit the editor body color unless an explicit `color` attribute is authored.
  `Subtitle` follows the `textBodyLg`-sized token contract, while `Footnote` follows the disabled text token contract.
- The canonical toolbar `style` attribute values are `Title`, `Title2`, `Subtitle`, `Header`, `Header2`, `Body`,
  `Description`, `Caption`, and `Footnote`. Empty or missing `style` attributes normalize to `Body`; inserting `Body`
  therefore uses the bare `<style>` opening token.
- Explicit attributes (`font`, `weight`, `size`, `color`, `background`, `align`, and existing `height`) override or
  extend the token projection when editor HTML is generated. `font` values are emitted as quoted CSS `font-family`
  strings so Qt RichText keeps the selected family instead of falling back to the editor default.
- A font-only source token such as `<style font="American Typewriter">` projects and matches as a family-only span.
  It must not require the implicit Body token size, weight, or line-height when recovering Qt/LVRS rich-text HTML,
  because those serializers may preserve only the selected `font-family` declaration for that run.
- Toolbar font-family values are normalized and escaped here before `SetTag` creates `<style font="...">` source
  wrappers.
- Editor projection emits `<!--whatson-style-source:...-->` metadata plus a styled `<span>` so the original opening
  token survives rich-text editing.
- Recovery helpers recognize the marker/anchor fallback shapes that `QTextDocument::toHtml()` may preserve, compare
  generated spans against the original opening token, and provide the formatting baseline used by persistence.

## Build Contract

- Registered by the recursive editor model CMake shard.
- The C++ regression target links this file explicitly because the test target does not consume the app target's
  recursive source registration.
