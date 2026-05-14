# `src/app/models/clipboard/InAppClipboardManager.cpp`

## Responsibility

Implements system clipboard capture, URL/file import orchestration, and resource package import for the QML
`inAppClipboard` context object.

## Notes

- Reads local file URLs, supported MIME payloads, text/html payloads, images, pixmaps, and image data URLs from
  `QClipboard`/`QMimeData`.
- Refreshes the resource availability snapshot from the current system clipboard instead of trusting a stale in-app
  snapshot.
- Accepts app-internal local files, raw bytes, and text as resource payloads.
- Delegates the single current clipboard resource snapshot to `InAppClipboardStore`.
- Persists local files or materialized clipboard payloads into `.wsresources/<id>.wsresource`.
- Updates `Resources.wsresources`, handles duplicate import policy, and returns editor insertion metadata.
- `ClipboardResourcePackageImport.cpp` must not be reintroduced; the package import pipeline is part of this object.

## 한국어

- 시스템 clipboard에서 앱이 리소스로 받아들일 수 있는 항목을 캡처한다.
- paste 직전 refresh는 기존 snapshot이 남아 있어도 현재 OS clipboard를 다시 캡처한다.
- 이미지 전용이 아니며, 앱 내부에서 전달하는 local file, raw bytes, text payload도 같은 경로로 처리한다.
- 현재 붙여넣기 후보 하나의 저장 상태는 `InAppClipboardStore`가 소유한다.
- `.wsresource` 패키지 생성, `Resources.wsresources` 갱신, 충돌 처리는 manager가 조율한다.
