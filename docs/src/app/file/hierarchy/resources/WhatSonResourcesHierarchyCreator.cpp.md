# `src/app/file/hierarchy/resources/WhatSonResourcesHierarchyCreator.cpp`

## Responsibility

`Resources.wsresources` 목록 파일을 직렬화한다.

이제 `resources` 배열은 단순 문자열 배열이 아니라 각 엔트리를
`{"resourcePath": "Hub.wsresources/id.wsresource"}` 형태의 object로 쓴다.

## Why

리소스의 canonical identity가 raw asset file이 아니라 `.wsresource` 패키지 경로가 되었기 때문이다.
이 포맷은 이후 메타데이터 확장에도 유리하고, 기존 string-array 파서와도 하위 호환을 유지한다.
