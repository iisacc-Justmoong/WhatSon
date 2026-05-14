# `src/app/models/clipboard/FiletypeCapture.h`

## Responsibility

Declares clipboard file type capture helpers used before a payload becomes a `ClipboardResourceImport`.

## Contract

- Normalizes MIME type strings.
- Maps MIME types to resource file formats.
- Resolves a file format from a file name first and MIME type second.
- Produces default file names for in-memory clipboard payloads.

## 한국어

- clipboard payload의 파일 형식 확인 책임은 이 객체에 있다.
- `ClipboardResourceImport`는 이 객체가 산출한 format을 받아 resource descriptor를 구성한다.
