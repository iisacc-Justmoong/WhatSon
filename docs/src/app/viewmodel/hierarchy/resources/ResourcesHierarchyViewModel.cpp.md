# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp`

## Responsibility

이 구현은 리소스 사이드바를 static taxonomy가 아니라 실제 `.wsresource` 메타데이터 트리로 노출한다.

## Tree Contract

`setResourcePaths(...)`는 이제 path list만 저장하지 않는다. 각 리소스 참조를 materialize해서:

- `bucket`
- `format`
- `asset`

3단 flattened row 벡터를 다시 만든다.

## Expansion Preservation

rebuild 시에는 이전 row들의 `key -> expanded` 상태를 복원한다.
그래서 런타임 스냅샷 갱신 이후에도 같은 bucket/format이 접힌 채나 펼쳐진 채로 유지된다.

## Load Fallback

`loadFromWshub(...)`는 우선 `.wscontents/Resources.wsresources`를 읽고,
그 파일이 비어 있거나 없으면 허브 루트의 `*.wsresources` 디렉터리를 직접 스캔해 flat `.wsresource` 패키지 목록을 만든다.

즉 리소스 도메인의 기준 경로는 항상 `.wsresource` 패키지 경로다.
