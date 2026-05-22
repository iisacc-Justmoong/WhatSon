# `src/app/models/editor/component/style.h`

## Responsibility

Declares the editor-domain component contract for the proprietary `.wsnbody` `<style ...>...</style>` wrapper.

## Public Contract

- `Style::openingToken()` and `Style::closingToken()` provide the paired static wrapper consumed by `SetTag`.
- `StyleToken` captures the LVRS text token metrics used by editor projection.
- `StyleSourceBaseline` captures the formatting baseline used when rich editor spans are converted back to canonical
  source tags.
- Attribute parsing and CSS projection helpers stay here so note-body persistence can delegate style policy instead of
  owning the component implementation inline.

## Build Contract

- Registered by the recursive `src/app/models/editor/CMakeLists.txt` shard.
- Covered by `test/cpp/suites/style_component_tests.cpp`.
