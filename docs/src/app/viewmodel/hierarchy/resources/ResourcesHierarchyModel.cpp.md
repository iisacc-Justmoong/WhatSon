# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyModel.cpp`

## Responsibility

리소스 하이어라키 row model은 단순 label/depth만 노출하지 않는다.

이제 각 row는 다음 메타데이터 role도 가진다.

- `count`
- `key`
- `kind`
- `bucket`
- `type`
- `format`
- `resourceId`
- `resourcePath`
- `assetPath`

## Row Kinds

- `type`
- `format`

`ShowChevronRole`도 더 이상 depth 비교로 추론하지 않고, 뷰모델이 계산한 구조값을 그대로 사용한다.
이 덕분에 `type -> format` 동적 트리가 안정적으로 유지된다.
