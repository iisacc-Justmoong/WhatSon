# `src/app/callout/ContentsCalloutBackend.hpp`

## Responsibility
Declares the callout editor backend bridge exposed to QML as
`WhatSon.App.Internal/ContentsCalloutBackend`.

## Public QML Contract
- `parseCallouts(sourceText)`: parses canonical `<callout>...</callout>` source blocks into render-model entries.
- `buildCalloutInsertionPayload(bodyText)`: builds canonical callout insertion source and cursor offset data.
- `detectCalloutEnterReplacement(sourceText, sourceStart, sourceEnd, insertedText)`: detects the second `Enter`
  pressed on a trailing empty callout line and returns an exit rewrite payload that moves editing outside the
  `</callout>` wrapper.

## Registration Constraint
- This QObject type is registered via `qmlRegisterType<ContentsCalloutBackend>()`.
- The class must remain non-`final` to keep compatibility with Qt's internal `QQmlElement<T>` wrapper type.
