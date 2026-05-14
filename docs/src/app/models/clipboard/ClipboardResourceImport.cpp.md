# `src/app/models/clipboard/ClipboardResourceImport.cpp`

## Responsibility

Implements clipboard import descriptor construction after file type capture has completed.

## Notes

- Delegates MIME and file suffix detection to `FiletypeCapture`.
- Applies type and bucket inference from `WhatSonResourcePackageSupport`, keeping clipboard imports aligned with the
  rest of the resource hierarchy after the file format is known.
- Provides default clipboard file names so in-memory payloads can be materialized before package creation.

## 한국어

- clipboard 후보의 파일 형식 확인은 `FiletypeCapture`에 맡긴다.
- 이 파일은 확인된 format을 앱이 지원하는 resource type/bucket으로 연결해 import descriptor를 만든다.
- `.mp3`, `.m4a`, `.flac` 같은 음악 파일 확장자도 별도 `music` type으로 나누지 않고 canonical
  `audio`/`Audio`로 정규화한다.
