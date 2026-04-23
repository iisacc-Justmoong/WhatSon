# `ContentsStructuredDocumentCollectionPolicy.cpp`

- Normalizes structured-document entry collections that arrive from both C++ list models and QML/JS array bindings.
- `normalizeEntries()` now treats `QJSValue` arrays as first-class list inputs by converting them through
  `QJSValue::toVariant()` before the existing `QVariantList` / indexed-map normalization rules run.
- This guards the structured editor against the historical blank-body regression where
  `ContentsStructuredDocumentFlow.qml` rebuilt `documentHost.documentBlocks` from a JS array, but the collection
  policy re-read that value as an empty collection and collapsed the document host to `blockCount=0`.
- Indexed-map normalization is still preserved for existing `"0"`, `"1"`, ... shaped payloads, so resource/block
  helper code keeps deterministic ordering across both C++ and QML producers.
