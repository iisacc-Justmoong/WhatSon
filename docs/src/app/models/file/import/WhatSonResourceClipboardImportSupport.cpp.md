# `src/app/models/file/import/WhatSonResourceClipboardImportSupport.cpp`

## Responsibility

Implements clipboard-image extraction for resource import.

## Supported Payloads

- Direct image MIME data such as `image/png`.
- Platform image-like MIME names such as `public.png` or `public.tiff`.
- HTML or text `data:image/...` payloads.
- `QMimeData::imageData()` values convertible to `QImage` or `QPixmap`.
- Direct `QClipboard::image()` / `QClipboard::pixmap()` fallback for native pasteboard image objects.

## Testing

`test/cpp/suites/resources_import_controller_tests.cpp` verifies platform MIME payload extraction and image-object
payload extraction.

## 한국어

- 이 구현은 clipboard image detection을 `ResourcesImportController`에서 분리해 유지한다.
- 스크린샷 캡처처럼 pasteboard가 MIME byte array 대신 image object로 제공하는 경우도 import 가능해야 한다.
