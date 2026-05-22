# `src/app/models/editor/component/style.cpp`

## Responsibility

Implements the editor-domain projection and recovery helpers for the proprietary `.wsnbody` `<style ...>` wrapper.

## Current Rules

- LVRS text tokens such as `Title`, `Title2`, `Header`, `Header2`, `Body`, `Description`, `Caption`, and `Disabled`
  map to fixed Pretendard size, weight, line-height, and color values.
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
