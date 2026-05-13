# `src/app/models/clipboard/ClipboardResourcePackageImport.cpp`

## Responsibility

Implements the resource package import shard for `InAppClipboard`.

## Flow

1. Resolve the current hub `.wscontents` and `.wsresources` paths.
2. Normalize selected local files or materialized clipboard payloads.
3. Detect duplicate resource asset names and apply Abort, Overwrite, or Keep Both policy.
4. Create or replace `.wsresource` packages, write metadata and annotation bitmap, and update `Resources.wsresources`.
5. Return editor insertion metadata when the caller is the note editor paste path.

## Guardrails

- Clipboard payloads are materialized into temporary local files before entering the same resource package pipeline as
  file imports.
- The note body receives only a resource path reference, never image bytes or an asset file path.
- Runtime reload is optional for editor paste; the editor path can insert metadata first and explicitly request resource
  reload afterward.

## 한국어

- 이 구현 파일은 별도 clipboard 객체가 아니라 `InAppClipboard`의 `.wsresource` 패키지 import 구현 shard다.
- `InAppClipboard`의 file URL import와 clipboard resource paste를 같은 패키지 생성 경로로 처리한다.
- clipboard 이미지도 먼저 `.wsresources/<id>.wsresource`에 등록되고, 본문에는 `<resource ...>` 참조만 삽입된다.
