# `src/app/models/file/import/WhatSonResourceClipboardImportSupport.hpp`

## Responsibility

Declares clipboard-image extraction helpers for the resource import pipeline.

## Contract

- `extractClipboardImage(QMimeData*, QImage*)` reads image payloads from MIME data.
- `extractClipboardImage(QClipboard*, QImage*)` first inspects MIME data, then falls back to direct clipboard image and
  pixmap APIs.
- `clipboardContainsImportableImage()` returns the current application clipboard image availability snapshot.
- The helper owns MIME/data-url/platform-pasteboard image detection policy so `ResourcesImportController` stays focused
  on import orchestration and persistence.

## 한국어

- 이 헤더는 clipboard image payload 추출 경계를 선언한다.
- macOS screenshot처럼 일반 `image/*` MIME이 아닌 pasteboard image object도 구현에서 처리할 수 있게
  `QClipboard` fallback API를 제공한다.
