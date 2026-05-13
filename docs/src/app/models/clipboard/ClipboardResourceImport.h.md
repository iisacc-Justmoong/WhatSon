# `src/app/models/clipboard/ClipboardResourceImport.h`

## Responsibility

Declares the lightweight value object used to describe one importable clipboard resource.

## Contract

- Stores the source file name, optional local file path, MIME type, normalized resource format, type, bucket, and
  optional in-memory payload.
- Provides helpers to build imports from file names, local files, images, and raw bytes.
- Uses the resource package taxonomy so clipboard images, PDFs, audio files, 3D models, archives, and other supported
  formats resolve to the same type/bucket labels as imported files.

## 한국어

- clipboard에서 얻은 단일 리소스를 설명하는 값 객체다.
- 파일명과 MIME type을 앱의 resource format/type/bucket 목록에 맞춰 정규화한다.
- payload 자체를 본문에 넣지 않고, 이후 import 단계가 `.wsresource` package로 materialize할 수 있는 정보만 담는다.
