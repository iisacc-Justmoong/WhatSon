# `src/app/models/clipboard/FiletypeCapture.cpp`

## Responsibility

Implements MIME and file-name based resource format detection for clipboard imports.

## Notes

- Keeps the MIME-to-format table out of `ClipboardResourceImport`.
- Uses `WhatSonResourcePackageSupport` format normalization so clipboard paste and hub resource imports share the same
  format vocabulary.
- Falls back unknown image MIME families to `.png` and generic octet-stream payloads to `.bin`.

## 한국어

- MIME type과 파일명 확장자에서 hub resource format을 산출한다.
- `ClipboardResourceImport`는 이 파일의 결과를 사용하지만, MIME table 자체를 소유하지 않는다.
