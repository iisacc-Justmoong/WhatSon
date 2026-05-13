# `src/app/models/clipboard/InAppClipboard.cpp`

## Responsibility

Implements single-item in-app clipboard state, system clipboard capture, and resource package import.

## Notes

- Reads local file URLs, supported MIME payloads, text/html payloads, images, pixmaps, and image data URLs from
  `QClipboard`/`QMimeData`.
- Accepts app-internal local files, raw bytes, and text as resource payloads.
- Stores at most one `ClipboardResourceImport`.
- Persists local files or materialized clipboard payloads into `.wsresources/<id>.wsresource`.
- Updates `Resources.wsresources`, handles duplicate import policy, and returns editor insertion metadata.
- `ClipboardResourcePackageImport.cpp` must not be reintroduced; the package import pipeline is part of this object.

## 한국어

- 시스템 clipboard에서 앱이 리소스로 받아들일 수 있는 단일 항목을 캡처한다.
- 이미지 전용이 아니며, 앱 내부에서 전달하는 local file, raw bytes, text payload도 같은 경로로 보관한다.
- 여러 항목을 저장하지 않고 현재 붙여넣기 후보 하나만 유지한다.
- `.wsresource` 패키지 생성, `Resources.wsresources` 갱신, 충돌 처리는 `InAppClipboard.cpp` 안에서 직접
  처리한다.
