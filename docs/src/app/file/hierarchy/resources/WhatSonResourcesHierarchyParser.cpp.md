# `src/app/file/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp`

## Responsibility

`Resources.wsresources`를 읽어 `.wsresource` 패키지 경로 목록으로 복원한다.

## Accepted Input Forms

- legacy string array
- object root with `resources: [...]`
- new object array entries with `resourcePath`
- legacy object array entries with `path` (대소문자 무시)
- root object direct path key (`resourcePath`/`path`)
- XML-like `<resource ... path=...>` 또는 `<resource ... resourcePath=...>` tag text
- 마지막 fallback으로 line-based text

## Compatibility

쓰기 포맷은 object array로 이동했지만, 파서는 기존 raw string 저장본도 그대로 읽는다.
따라서 기존 허브의 `Resources.wsresources`를 즉시 마이그레이션하지 않아도 런타임 적재가 깨지지 않는다.
`<resources>` wrapper 같은 markup 라인은 fallback 단계에서 경로로 취급하지 않고 건너뛴다.
