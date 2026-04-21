# `src/app/models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp`

## Responsibility

이 헤더는 단수형 `.wsresource` 패키지 규약의 기준 구현이다.

- 패키지 디렉터리 suffix: `.wsresource`
- 메타데이터 파일 이름: `resource.xml`
- 기본 주석 캔버스 파일 이름: `annotation.png`
- 메타데이터 루트: `<wsresource ...><annotation path="annotation.png"/><asset path="..."/></wsresource>`

## Metadata Contract

`ResourcePackageMetadata`는 다음 값을 정규화한다.

- `resourceId`
- `resourcePath`
- `assetPath`
- `annotationPath`
- `bucket`
- `type`
- `format`

메타데이터의 `assetPath`는 패키지 내부의 실제 원본 에셋 파일을 가리키며, `resourcePath`는 허브 기준 경로
예를 들어 `Hub.wsresources/logo.wsresource` 형식을 유지한다.
`annotationPath`는 패키지 내부 주석 오버레이 bitmap 경로를 가리키며, 기본값은 항상 `annotation.png`다.

## Runtime Helpers

이 헤더는 네 종류의 런타임 보조 함수를 제공한다.

- `buildMetadataForAssetFile(...)`
- `createEmptyAnnotationBitmap(...)` / `createEmptyAnnotationBitmapPngBytes(...)` / `writeResourcePackageAnnotationBitmap(...)`
- `createResourcePackageMetadataXml(...)` / `parseResourcePackageMetadataXml(...)`
- `loadResourcePackageMetadata(...)`
- `resolveAssetLocationFromReference(...)`

`buildMetadataForAssetFile(...)`는 파일 경로 하나만 받아:

- `format`은 실제 파일명이 가진 suffix를 그대로 보존하고
- `type`과 `bucket`은 그 suffix를 case-insensitive로 해석해 자동 할당한다

예를 들어 `VisualAsset.PNG`는 `format=".PNG"`, `type="image"`, `bucket="Image"`가 된다.
같은 호출은 새 패키지 주석 캔버스 경로도 `annotation.png`로 기본 설정한다.

`createEmptyAnnotationBitmap(...)` 계열은 package-local 주석 캔버스를 생성한다.

- 원본 asset이 bitmap 이미지면 그 실제 픽셀 크기를 따라가는 투명 PNG를 만든다.
- 이미지 크기를 알아낼 수 없으면 최소 `1x1` 투명 PNG로 fallback 한다.
- `writeResourcePackageAnnotationBitmap(...)`는 그 PNG를 `annotation.png`로 패키지 내부에 원자적으로 기록한다.

마지막 함수는 `.wsnbody`의 `<resource ... resourcePath="...">`와 같은 참조를 받아:

- 패키지 디렉터리면 `resource.xml`을 읽고 내부 에셋 파일 절대 경로로 변환하고
- 기존 raw 파일/URL 참조면 그대로 호환 처리한다.

## Hub Storage Rules

단일 루트 집계 함수와 함께 허브 단위 멀티 루트 집계 함수가 제공된다.

- `resolveResourceRootDirectories(...)`: 허브 내부 `.wsresources` + `*.wsresources` 디렉터리를 모두 반환
- `listRelativeResourcePackagePaths(...)`: 특정 리소스 루트 1개에서 `.wsresource`를 집계
- `listRelativeResourcePackagePathsForHub(...)`: 허브의 모든 리소스 루트를 순회해 상대 경로를 중복 제거 후 집계
- `countResourcePackages(...)`: 단일 루트 기준 개수 집계

이 규칙으로 리소스가 여러 루트로 분산된 허브에서도 런타임/뷰모델이 동일한 패키지 집합을 재구성할 수 있다.
