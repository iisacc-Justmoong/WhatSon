# `src/app/models/editor/EditorFontFamilyProvider.hpp`

`EditorFontFamilyProvider` exposes font-family menu data.

It does not apply font tags by itself. The previous active editor document command path was deleted, so callers must not assume this provider can mutate editor source.
