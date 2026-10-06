# `src/app/models/sensor/UnusedResourcesSensor.hpp`

<a id="responsibility"></a>

## 책임
허브에 있지만 메모 본문에 포함되지 않은 리소스 패키지를 보고하는 QObject 센서를 선언합니다.

<a id="public-surface"></a>

## 공공 표면
- `hubPath`: 스캔 루트가 되는 압축을 푼 `.wshub` 디렉터리.
- `unusedResources`: 사용되지 않은 패키지 설명자의 `QVariantList`입니다.
- `unusedResourcePaths`: `unusedResources`에서 추출된 편의 `QStringList` 투영입니다.
- `unusedResourceCount`: 목록/세부 패널 또는 진단을 위한 카운트 투영.
- `lastError`: 가장 최근의 검증 또는 스캔 오류입니다.
- `scanUnusedResources(...)`: 센서 상태를 새로 고치고 사용되지 않은 항목을 반환합니다.
- `collectUnusedResourcePaths(...)`: 리소스 경로만 반환하는 편리한 새로 고침 경로입니다.
- `refresh()`: 파일 시스템 변경 후 상위 계층이 스캔을 다시 실행할 수 있도록 명시적인 슬롯 진입점입니다.

<a id="signals"></a>

## 신호
- `hubPathChanged()`
- `unusedResourcesChanged()`
- `lastErrorChanged()`
- `scanCompleted(...)`
