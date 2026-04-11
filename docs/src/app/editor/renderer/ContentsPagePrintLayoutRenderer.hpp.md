# `src/app/editor/renderer/ContentsPagePrintLayoutRenderer.hpp`

## Responsibility

Declares the page/print layout renderer bridge used by QML editor surfaces.
The renderer centralizes page/print mode gating and paper geometry calculations so the QML layer no longer owns
Page/Print-specific math directly.

## Public Contract

- Mode inputs
  - `activeEditorViewMode`
  - `hasSelectedNote`
  - `dedicatedResourceViewerVisible`
- Layout inputs
  - `editorViewportWidth`
  - `editorViewportHeight`
  - `editorContentHeight`
  - `guideHorizontalInset`
  - `guideVerticalInset`
  - `paperAspectRatio`
  - `paperHorizontalMargin`
  - `paperVerticalMargin`
  - `paperMaxWidth`
  - `paperSeparatorThickness`
  - `paperShadowOffsetX`
  - `paperShadowOffsetY`
- Mode outputs
  - `pageViewModeValue`
  - `printViewModeValue`
  - `showPageEditorLayout`
  - `showPrintModeActive`
  - `showPrintEditorLayout`
  - `showPrintMarginGuides`
- Paper geometry outputs
  - `documentPageCount`
  - `paperResolvedWidth`
  - `paperResolvedHeight`
  - `paperTextWidth`
  - `paperTextHeight`
  - `paperDocumentHeight`
  - `documentSurfaceHeight`
- Paper visual tokens
  - `canvasColor`
  - `paperBorderColor`
  - `paperColor`
  - `paperHighlightColor`
  - `paperShadeColor`
  - `paperSeparatorColor`
  - `paperShadowColor`
  - `paperTextColor`
- Hook slot
  - `requestRefresh()`

## Signals

- `modeStateChanged`
- `layoutStateChanged`

These two signals are the authoritative update contract for QML bindings that consume page/print mode and layout state.

## Architectural Constraint

- This type is registered through `qmlRegisterType(...)`, so it must remain subclassable by Qt's internal
  `QQmlElement<T>` wrapper and therefore must not be declared `final`.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - `WhatSonQmlInternalTypeRegistrar` must register `ContentsPagePrintLayoutRenderer` without Qt template-instantiation
    errors related to `QQmlElement<T>` inheritance.
