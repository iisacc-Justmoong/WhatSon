# `src/app/models/clipboard/InAppClipboardStore.h`

## Responsibility

Declares the QObject store that owns the current in-app clipboard resource snapshot.

## Contract

- Owns the `ClipboardResourceImport m_resourceImport` member.
- Provides read-only accessors used by `InAppClipboardManager`.
- Supports `setResourceImport(...)`, `takeResourceImport()`, and `clear()`.
- Exposes `resourceChanged()` so the manager can forward state changes to QML.

## 한국어

- `InAppClipboardManager`의 store 책임은 이 객체에 있다.
- `ClipboardResourceImport` 멤버는 manager가 아니라 이 store에만 남아야 한다.
- import orchestration, hub path, busy/error 상태는 store가 아니라 manager에 남는다.
