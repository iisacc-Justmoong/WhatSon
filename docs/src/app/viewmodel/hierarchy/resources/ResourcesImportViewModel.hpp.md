# `src/app/viewmodel/hierarchy/resources/ResourcesImportViewModel.hpp`

## Role

이 ViewModel은 ApplicationWindow 전역 파일 드래그 앤 드랍을 리소스 패키지 import로 번역한다.

- 현재 활성 `.wshub` 경로를 보관하고
- 외부에서 드롭된 파일 URL이 import 가능한지 판정하며
- 실제 import 수행 뒤 런타임 리로드 콜백까지 호출한다

## Public Contract

- `currentHubPath`
  현재 import 대상 허브다. 비어 있으면 드롭 import는 거부된다.
- `busy`
  import와 후속 리로드가 진행 중임을 나타낸다.
- `lastError`
  마지막 실패 메시지다.

## Hooks And Signals

- `canImportDroppedUrls(...)`
  QML `DropArea`가 오버레이 표시 여부를 결정할 때 쓴다.
- `importDroppedUrls(...)`
  실제 파일 import entrypoint다.
- `importCompleted(int)`
  성공 시 import된 파일 수를 알린다.
- `operationFailed(QString)`
  저장 또는 런타임 반영 실패를 알린다.

## Storage Policy

드롭된 각 파일은 현재 허브의 `*.wsresources` 아래에 다음 규칙으로 들어간다.

- 고유한 `resourceId`를 만든다
- `resourceId.wsresource` 디렉터리를 flat하게 만든다
- 원본 파일명은 그대로 복사한다
- `resource.xml`은 `buildMetadataForAssetFile(...)` 기준으로 기록한다
- `Resources.wsresources` 목록 파일에 relative package path를 append한다
