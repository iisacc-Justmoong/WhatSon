# `src/app/models/editor/tags/ContentsEditorTagMutationBuilder.cpp`

## Responsibility

Implements shortcut-independent RAW `.wsnbody` tag mutation payload construction.

## Current Behavior

- `normalizedTagName(...)` accepts aliases and canonicalizes supported inline/body tag names.
- `buildTagInsertionPayload(...)` normalizes source text and delegates to either generated body-tag insertion or paired
  tag wrapping.
- Inline formatting wraps first normalize the selected RAW range against existing inline-style tag tokens. A selection
  that starts or ends inside `<highlight>` / `</highlight>` or includes only one side of an existing inline wrapper is
  moved to the visible content boundary before the next RAW mutation is built.
- A non-empty selection such as `Alpha` with `bold` becomes `<bold>Alpha</bold>`.
- A collapsed cursor inserts an empty pair such as `<bold></bold>` and returns a cursor position between the tags.
- Collapsed callout and agenda commands insert plain paired tags at the cursor:
  `<callout></callout>` and `<agenda><task></task></agenda>`.
- Collapsed generated body-tag commands are limited to break insertion (`</break>`).
- The returned payload carries `nextSourceText`, `selectionStart`, `selectionEnd`, `cursorPosition`, `tagName`,
  `insertedSourceText`, and optional wrapped/replacement text fields.

## Boundary

The builder treats formatting tags and transparent paired tags as the same RAW insertion category. Specialized break
helpers may still generate canonical source snippets, but they must feed the same next-source mutation path. Inline
formatting may inspect existing RAW tag tokens only to preserve balanced source boundaries; it must not serialize the
rendered RichText/iiHtmlBlock projection back into `.wsnbody`.

## 한국어

이 구현은 단축키를 모르고 tag 이름과 RAW selection만으로 다음 `.wsnbody` source snapshot을 만든다. 따라서 단축키
외의 UI도 동일한 태그 생성 정책을 재사용할 수 있다.
