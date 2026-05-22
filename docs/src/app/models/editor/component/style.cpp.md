# `src/app/models/editor/component/style.cpp`

## Responsibility

Implements the editor-domain projection and recovery helpers for the proprietary `.wsnbody` `<style ...>` wrapper.

## Current Rules

- LVRS text tokens such as `Title`, `Title2`, `Subtitle`, `Header`, `Header2`, `Body`, `Description`, `Caption`,
  `Footnote`, and legacy `Disabled` map to the app's LVRS token-aligned Pretendard size, weight, line-height, and
  color values. The mapping deliberately mixes typography and color tokens so each semantic style has a stronger
  preview personality: `Title` uses the primary accent, `Title2` uses purple, `Subtitle` uses accent blue, `Header`
  uses success green, and `Header2` uses muted yellow while Body/Description/Caption/Footnote remain on the text color
  ramp. `Subtitle` follows the `textBodyLg`-sized token contract, while `Footnote` follows the disabled text token
  contract.
- The canonical toolbar `style` attribute values are `Title`, `Title2`, `Subtitle`, `Header`, `Header2`, `Body`,
  `Description`, `Caption`, and `Footnote`. Empty or missing `style` attributes normalize to `Body`; inserting `Body`
  therefore uses the bare `<style>` opening token.
- Explicit attributes (`font`, `weight`, `size`, `color`, `background`, `align`, and existing `height`) override or
  extend the token projection when editor HTML is generated.
- Editor projection emits `<!--whatson-style-source:...-->` metadata plus a styled `<span>` so the original opening
  token survives rich-text editing.
- Recovery helpers recognize the marker/anchor fallback shapes that `QTextDocument::toHtml()` may preserve, compare
  generated spans against the original opening token, and provide the formatting baseline used by persistence.

## Build Contract

- Registered by the recursive editor model CMake shard.
- The C++ regression target links this file explicitly because the test target does not consume the app target's
  recursive source registration.
