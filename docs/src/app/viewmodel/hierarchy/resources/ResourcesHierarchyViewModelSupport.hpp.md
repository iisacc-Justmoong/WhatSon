# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp`

## Responsibility

리소스 하이어라키 support 헤더는 세 가지를 맡는다.

- `Resources.wsresources`와 `.wshub` contents 경로 해석
- `.wsresource` 메타데이터 materialize
- materialized entry를 `bucket -> format -> asset` flattened row로 변환

## Materialization Rule

각 `resourcePath`는 먼저 `WhatSonResourcePackageSupport.hpp`를 통해 해석된다.

- 패키지 메타데이터가 정상적이면 그 값을 사용하고
- 그렇지 않으면 legacy raw path에서 fallback metadata를 만든다

이 단계에서 `resourceId`, `bucket`, `type`, `format`, `assetPath`가 확정된다.

## Stable Keys

확장 상태 복원을 위해 row key를 명시적으로 만든다.

- `bucket:image`
- `format:image:.png`
- `asset:Hub.wsresources/logo.wsresource`

이 key가 `ResourcesHierarchyViewModel`의 expansion-preserve 동작의 기준이다.
포맷 key는 원본 메타데이터 포맷 표기와 별개로 case-folded lookup 값을 써서, `.PNG`와 `.png`가 같은 분류 노드로 합쳐진다.

## Empty-State Fallback

When `resourcePaths` is empty, `buildHierarchyItems(...)` now returns a non-empty default bucket list
instead of an empty hierarchy.

- `Image`
- `Video`
- `Document`
- `3D Model`
- `Web page`
- `Music`
- `Audio`
- `ZIP`
- `Other`

These fallback rows are bucket-only nodes (`kind="bucket"`, `showChevron=false`) so the sidebar keeps
a stable visual structure even before the first resource import.
