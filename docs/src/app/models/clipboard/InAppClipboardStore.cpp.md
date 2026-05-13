# `src/app/models/clipboard/InAppClipboardStore.cpp`

## Responsibility

Implements the single-resource clipboard snapshot store used by `InAppClipboardManager`.

## Notes

- Stores at most one `ClipboardResourceImport`.
- Exposes normalized resource file name, MIME type, format, type, bucket, and variant-map metadata.
- Emits `resourceChanged()` when a valid resource is stored, taken, or cleared.
- Clears itself when an invalid resource import is assigned.

## 한국어

- manager가 들고 있던 현재 clipboard resource 상태를 이 store가 소유한다.
- 여러 항목을 저장하지 않고 현재 붙여넣기 후보 하나만 유지한다.
- manager는 이 store의 `resourceChanged()` 신호를 QML-facing `resourceChanged()`로 다시 전달한다.
