# `src/app/models/clipboard/InAppClipboardStore.cpp`

<a id="responsibility"></a>

## 책임

`InAppClipboardManager`에서 사용하는 단일 리소스 클립보드 스냅샷 저장소를 구현합니다.

<a id="notes"></a>

## 메모

- 최대 하나의 `ClipboardResourceImport`를 저장합니다.
- 정규화된 리소스 파일 이름, MIME 유형, 형식, 유형, 버킷 및 변형 맵 메타데이터를 노출합니다.
- 유효한 리소스가 저장, 가져오기 또는 삭제되면 `resourceChanged()`를 내보냅니다.
- 잘못된 리소스 가져오기가 할당되면 자체적으로 지워집니다.

## 한국어

- manager가 들고 있던 현재 clipboard resource 상태를 이 store가 소유한다.
- 여러 항목을 저장하지 않고 현재 붙여넣기 후보 하나만 유지한다.
- manager는 이 store의 `resourceChanged()` 신호를 QML-facing `resourceChanged()`로 다시 전달한다.
