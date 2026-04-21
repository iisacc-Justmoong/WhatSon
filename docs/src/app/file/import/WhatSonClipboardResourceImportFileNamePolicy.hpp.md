# `src/app/models/file/import/WhatSonClipboardResourceImportFileNamePolicy.hpp`

## Role
This header exposes the clipboard-resource asset naming policy used by the import pipeline.

## Public Contract

- `WhatSon::Resources::generateClipboardImportAssetFileName()`
  Returns a `.png` asset file name whose stem is a random 32-character mixed-case alphanumeric key.
  Clipboard-originated imports use this policy before the resource package copy begins.
