# `src/app/models/clipboard/InAppClipboardStore.h`

<a id="responsibility"></a>

## 책임

현재 인앱 클립보드 리소스 스냅샷을 소유하는 QObject 저장소를 선언합니다.

<a id="contract"></a>

## 계약

- `ClipboardResourceImport m_resourceImport` 멤버를 소유한다.
- `InAppClipboardManager`에서 사용하는 읽기 전용 접근자를 제공합니다.
- `setResourceImport(...)`, `takeResourceImport()` 및 `clear()`를 지원합니다.
- 관리자가 상태 변경 사항을 QML에 전달할 수 있도록 `resourceChanged()`를 노출합니다.

## 한국어

- `InAppClipboardManager`의 store 책임은 이 객체에 있다.
- `ClipboardResourceImport` 멤버는 manager가 아니라 이 store에만 남아야 한다.
- import orchestration, hub path, busy/error 상태는 store가 아니라 manager에 남는다.
