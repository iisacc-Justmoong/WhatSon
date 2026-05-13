# `src/app/models/clipboard/ClipboardResourceImport.h`

## Responsibility

Declares the lightweight value object used to describe one importable clipboard resource.

## Contract

- Stores the source file name, optional local file path, MIME type, normalized resource format, type, bucket, and
  optional in-memory payload.
- Provides helpers to build imports from file names, local files, images, raw bytes, and text-backed payloads supplied
  through `InAppClipboard`.
- Uses the resource package taxonomy so clipboard images, PDFs, text/HTML documents, audio files, 3D models, archives,
  and other supported formats resolve to the same type/bucket labels as imported files.

## 한국어

- clipboard에서 얻은 단일 리소스를 설명하는 값 객체다.
- 파일명과 MIME type을 앱의 resource format/type/bucket 목록에 맞춰 정규화한다.
- 이미지뿐 아니라 문서, 텍스트/HTML, 오디오, 비디오, 3D 모델, 압축 파일 payload도 같은 값 객체로 표현한다.
- payload 자체를 본문에 넣지 않고, 이후 import 단계가 `.wsresource` package로 materialize할 수 있는 정보만 담는다.
