# `src/app/models/editor/component/style.h`

## Responsibility

Declares the editor-domain component contract for the proprietary `.wsnbody` `<style ...>...</style>` wrapper.

## Public Contract

- `Style::openingToken()` and `Style::closingToken()` provide the paired static wrapper consumed by `SetTag`.
- `Style::styleAttributeValues()`, `Style::normalizedStyleAttributeValue(...)`, and
  `Style::openingTokenForStyleAttributeValue(...)` own the canonical toolbar-facing `style` attribute values. `Body`
  is the fallback for an empty attribute and therefore serializes as `<style>` instead of `<style style="Body">`.
- `Style::normalizedFontFamilyAttributeValue(...)` and `Style::openingTokenForFontFamily(...)` own toolbar font
  selector value normalization and `<style font="...">` source token generation. CSS projection quotes that family
  value in the generated `font-family` declaration so multi-word system fonts render as the selected family in
  `QTextDocument`. A font-only source token is treated as family-only for projection and recovery; LVRS text-token
  metrics are required only when a `style` attribute is present or the bare `<style>` Body fallback is used.
- `Style::normalizedFontSizeAttributeValue(...)` and `Style::openingTokenForFontSize(...)` own toolbar font-size
  normalization and `<style size="...">` source token generation. The authored value must be a positive integer, and
  projection emits it as `font-size:<value>px`.
- `Style::normalizedFontWeightAttributeValue(...)` and `Style::openingTokenForFontWeight(...)` own toolbar font-weight
  normalization and `<style weight="...">` source token generation. `bold` normalizes to the explicit `900` value used
  by the editor bold command, while `regular`/`normal` normalize to `400`.
- `StyleToken` captures the LVRS text token metrics used by editor projection.
- `StyleSourceBaseline` captures the formatting baseline used when rich editor spans are converted back to canonical
  source tags.
- Attribute parsing and CSS projection helpers stay here so note-body persistence can delegate style policy instead of
  owning the component implementation inline.

## Build Contract

- Registered by the recursive `src/app/models/editor/CMakeLists.txt` shard.
- Covered by `test/cpp/suites/style_component_tests.cpp`.
