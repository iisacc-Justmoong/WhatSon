# `src/app/models/sensor/UnusedResourcesSensor.cpp`

<a id="responsibility"></a>

## 책임
압축이 풀린 허브 파일에 대해 사용되지 않은 리소스 검색을 구현합니다.

<a id="scan-strategy"></a>

## 스캔 전략
1. `hubPath`가 압축이 풀린 `.wshub` 디렉터리를 가리키는지 확인합니다.
2. 각 허브 리소스 루트 아래의 모든 `.wsresource` 패키지를 열거합니다.
3. 패키지 인벤토리에 대한 패키지 설명자를 반환합니다. 노트 본문 소스는 스캔되지 않습니다.

<a id="returned-entry-shape"></a>

## 반환된 항목 모양
- `resourcePath`
- `packageDirectoryPath`
- `packageName`
- `resourceId`
- `assetPath`
- `assetAbsolutePath`
- `annotationPath`
- `annotationAbsolutePath`
- `bucket`
- `type`
- `format`
- `metadataValid`
- `metadataError`

<a id="error-handling"></a>

## 오류 처리
- 잘못된 허브 루트 및 읽을 수 없는 메모 본문 파일은 `lastError`를 설정하고 사용되지 않은 리소스 목록을 지웁니다.
- 손상된 `resource.xml` 메타데이터는 센서에서 패키지를 숨기지 않습니다. 구현은 패키지 경로 추론으로 되돌아가 손상된 패키지가 사라지는 대신 사용되지 않은 리소스로 표시됩니다 아무런 알림 없이
