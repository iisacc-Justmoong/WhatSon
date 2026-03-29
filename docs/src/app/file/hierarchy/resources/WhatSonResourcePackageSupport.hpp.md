# `src/app/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp`

## Responsibility

이 헤더는 단수형 `.wsresource` 패키지 규약의 기준 구현이다.

- 패키지 디렉터리 suffix: `.wsresource`
- 메타데이터 파일 이름: `resource.xml`
- 메타데이터 루트: `<wsresource ...><asset path="..."/></wsresource>`

## Metadata Contract

`ResourcePackageMetadata`는 다음 값을 정규화한다.

- `resourceId`
- `resourcePath`
- `assetPath`
- `bucket`
- `type`
- `format`

메타데이터의 `assetPath`는 패키지 내부의 실제 원본 에셋 파일을 가리키며, `resourcePath`는 허브 기준 경로
예를 들어 `Hub.wsresources/logo.wsresource` 형식을 유지한다.

## Runtime Helpers

이 헤더는 네 종류의 런타임 보조 함수를 제공한다.

- `buildMetadataForAssetFile(...)`
- `createResourcePackageMetadataXml(...)` / `parseResourcePackageMetadataXml(...)`
- `loadResourcePackageMetadata(...)`
- `resolveAssetLocationFromReference(...)`

`buildMetadataForAssetFile(...)`는 파일 경로 하나만 받아:

- `format`은 실제 파일명이 가진 suffix를 그대로 보존하고
- `type`과 `bucket`은 그 suffix를 case-insensitive로 해석해 자동 할당한다

예를 들어 `VisualAsset.PNG`는 `format=".PNG"`, `type="image"`, `bucket="Image"`가 된다.

마지막 함수는 `.wsnbody`의 `<resource ... resourcePath="...">`와 같은 참조를 받아:

- 패키지 디렉터리면 `resource.xml`을 읽고 내부 에셋 파일 절대 경로로 변환하고
- 기존 raw 파일/URL 참조면 그대로 호환 처리한다.

## Hub Storage Rules

`listRelativeResourcePackagePaths(...)`와 `countResourcePackages(...)`는 허브 루트의
`*.wsresources` 디렉터리를 flat 저장소로 보고, 직계 자식 `.wsresource` 디렉터리만 집계한다.
