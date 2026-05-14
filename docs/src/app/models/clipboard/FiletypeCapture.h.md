# `src/app/models/clipboard/FiletypeCapture.h`

## Responsibility

Declares clipboard file type capture helpers used before a payload becomes a `ClipboardResourceImport`.

## Contract

- Normalizes MIME type strings.
- Maps MIME types to resource file formats.
- Detects platform image-payload MIME names such as `application/x-qt-image` and `com.apple.tiff` before the payload is
  materialized as a clipboard resource.
- Resolves a file format from a file name first and MIME type second.
- Produces default file names for in-memory clipboard payloads.

## 한국어

- clipboard payload의 파일 형식 확인 책임은 이 객체에 있다.
- macOS/Qt 스크린샷 clipboard가 쓰는 `application/x-qt-image`, `com.apple.tiff` 같은 platform image MIME 판별도
  이 객체의 책임이다.
- `ClipboardResourceImport`는 이 객체가 산출한 format을 받아 resource descriptor를 구성한다.
