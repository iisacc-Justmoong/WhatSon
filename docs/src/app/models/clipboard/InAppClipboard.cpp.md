# `src/app/models/clipboard/InAppClipboard.cpp`

## Responsibility

Implements single-item in-app clipboard state and system clipboard capture.

## Notes

- Reads local file URLs, supported MIME payloads, images, pixmaps, and image data URLs from `QClipboard`/`QMimeData`.
- Stores at most one `ClipboardResourceImport`.
- Leaves package persistence and duplicate handling to `InAppClipboardResourceImport.cpp`.

## 한국어

- 시스템 clipboard에서 앱이 리소스로 받아들일 수 있는 단일 항목을 캡처한다.
- 여러 항목을 저장하지 않고 현재 붙여넣기 후보 하나만 유지한다.
- `.wsresource` 패키지 생성과 충돌 처리는 같은 객체의 import 구현 파일에서 처리한다.
