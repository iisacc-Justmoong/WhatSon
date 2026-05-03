# `src/app/models/editor/minimap/ContentsMinimapLayoutMetrics.hpp`

## Role

Declares the C++ QObject that resolves editor minimap layout metrics for QML.

## Contract

- Receives LVRS metric tokens as integer properties.
- Receives runtime visibility and `logicalLineCount` from the editor host.
- Publishes read-only calculated metrics:
  - `defaultMinimapWidth`
  - `designRowCount`
  - `effectiveMinimapWidth`
  - `effectiveRowCount`
- Emits `metricsChanged()` whenever an input can affect the calculated output.

## Boundaries

The class does not render minimap rows, inspect editor viewport geometry, or persist editor state. It only maps
primitive editor and token inputs into minimap layout values.
