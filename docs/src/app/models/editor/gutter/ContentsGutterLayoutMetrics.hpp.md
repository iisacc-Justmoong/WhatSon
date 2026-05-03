# `src/app/models/editor/gutter/ContentsGutterLayoutMetrics.hpp`

## Role

Declares the C++ QObject that resolves editor gutter layout metrics for QML.

## Contract

- Receives LVRS metric tokens as integer properties.
- Receives runtime overrides from `ContentsDisplayView.qml`.
- Publishes read-only calculated metrics:
  - `defaultActiveLineNumber`
  - `defaultGutterWidth`
  - `designLineNumberCount`
  - `effectiveGutterWidth`
  - `effectiveLineNumberCount`
  - `changedMarkerHeight`
  - `changedMarkerY`
  - `conflictMarkerHeight`
  - `conflictMarkerY`
  - `iconRailX`
  - `inactiveLineNumber`
  - `lineNumberBaseOffset`
  - `lineNumberColumnLeft`
  - `lineNumberColumnTextWidth`
- Emits `metricsChanged()` whenever an input can affect the calculated output.

## Boundaries

The class does not parse `.wsnbody`, inspect QML geometry, or persist editor state. It only maps primitive editor and
token inputs into gutter layout values.
