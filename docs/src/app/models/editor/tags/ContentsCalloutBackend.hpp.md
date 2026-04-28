# `src/app/models/editor/tags/ContentsCalloutBackend.hpp`

## Responsibility
Declares the callout editor backend bridge exposed to QML as
`WhatSon.App.Internal/ContentsCalloutBackend`.

## Public QML Contract
- `lastParseVerification`: latest parser verification report for callout tags.
- `parseCallouts(sourceText)`: parses canonical `<callout>...</callout>` source blocks into render-model entries.
- `detectCalloutEnterReplacement(sourceText, sourceStart, sourceEnd, insertedText)`: turns a plain Enter request inside
  callout content into a source rewrite that closes the callout at the current source cursor and moves editing outside
  the `</callout>` wrapper.

## Signals
- `lastParseVerificationChanged()`: emitted when the cached callout parse verification report changes.
- `parseVerificationReported(verification)`: emitted on every callout parse pass with counts/issues for
  `<callout>` confirmation state.

## Registration Constraint
- This QObject type is registered through the LVRS manifest in `WhatSonQmlInternalTypeRegistrar`.
- The class must remain non-`final` to keep compatibility with Qt's internal `QQmlElement<T>` wrapper type.
