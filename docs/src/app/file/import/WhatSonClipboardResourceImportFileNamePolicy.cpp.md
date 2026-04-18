# `src/app/file/import/WhatSonClipboardResourceImportFileNamePolicy.cpp`

## Responsibility
This implementation centralizes clipboard-import asset naming so both production code and the C++ regression suite use
the same contract.

## Current Rules

- The generated stem length is exactly 32 characters.
- The allowed alphabet is `0-9`, `A-Z`, and `a-z`.
- The generated asset file name always ends with `.png`.
- The generator uses `QRandomGenerator::global()` so repeated clipboard imports do not reuse a fixed placeholder file
  name.
