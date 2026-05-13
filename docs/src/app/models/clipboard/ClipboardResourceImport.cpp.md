# `src/app/models/clipboard/ClipboardResourceImport.cpp`

## Responsibility

Implements resource taxonomy mapping for clipboard import candidates.

## Notes

- Normalizes MIME types and file suffixes into resource formats.
- Delegates type and bucket inference to `WhatSonResourcePackageSupport`, keeping clipboard imports aligned with the
  rest of the resource hierarchy.
- Provides default clipboard file names so in-memory payloads can be materialized before package creation.

## 한국어

- clipboard 후보의 MIME type과 파일 확장자를 resource format/type/bucket으로 정규화한다.
- 앱이 지원하는 리소스 목록과 같은 taxonomy를 사용하므로 clipboard 이미지, 문서, 오디오, 3D 모델 등이 파일 import와
  같은 분류로 들어간다.
