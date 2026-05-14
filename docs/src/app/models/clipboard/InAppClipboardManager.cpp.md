# `src/app/models/clipboard/InAppClipboardManager.cpp`

## Responsibility

Implements system clipboard capture, URL/file import orchestration, and resource package import for the QML
`inAppClipboard` context object.

## Notes

- Reads local file URLs, supported MIME payloads, text/html payloads, images, pixmaps, and image data URLs from
  `QClipboard`/`QMimeData`.
- Extracts platform image MIME payloads and image data URLs before generic text/html resource capture, so screenshot
  paste is imported as an image `.wsresource` instead of being skipped or stored as text/html.
- Refreshes the resource availability snapshot from the current system clipboard instead of trusting a stale in-app
  snapshot.
- Accepts app-internal local files, raw bytes, and text as resource payloads.
- Delegates the single current clipboard resource snapshot to `InAppClipboardStore`.
- Persists local files or materialized clipboard payloads into `.wsresources/<id>.wsresource`.
- Materialized clipboard payloads that use the default `clipboard-resource.*` temporary file name are stored with a
  fresh 32-character alphanumeric resource id and matching asset file name, so repeated screenshot paste does not hit
  duplicate file-name conflicts.
- Updates `Resources.wsresources`, handles duplicate import policy, and returns editor insertion metadata.
- `ClipboardResourcePackageImport.cpp` must not be reintroduced; the package import pipeline is part of this object.

## 한국어

- 시스템 clipboard에서 앱이 리소스로 받아들일 수 있는 항목을 캡처한다.
- platform image MIME payload와 image data URL은 일반 text/html 리소스 캡처보다 먼저 이미지로 추출한다.
- paste 직전 refresh는 기존 snapshot이 남아 있어도 현재 OS clipboard를 다시 캡처한다.
- 이미지 전용이 아니며, 앱 내부에서 전달하는 local file, raw bytes, text payload도 같은 경로로 처리한다.
- 현재 붙여넣기 후보 하나의 저장 상태는 `InAppClipboardStore`가 소유한다.
- `.wsresource` 패키지 생성, `Resources.wsresources` 갱신, 충돌 처리는 manager가 조율한다.
- 기본 임시 이름인 `clipboard-resource.*`로 materialize된 clipboard payload는 32자 영문대소숫자 resource id와
  같은 asset 파일명으로 저장해 반복 스크린샷 붙여넣기 간 이름 충돌을 만들지 않는다.
