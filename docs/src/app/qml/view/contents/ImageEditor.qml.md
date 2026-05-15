# `src/app/qml/view/contents/ImageEditor.qml`

## Role

`ImageEditor.qml` is the image-resource viewer mounted by `ContentViewLayout.qml` when the active list model exposes a
resource entry whose type or format is image-backed.

It is a presentation-only LVRS contents view. It consumes the selected resource entry map, resolves a QML image URL from
`source`, `resolvedPath`, or `resourcePath`, and renders the image with `Image.PreserveAspectFit`.

## Boundary

- The view does not import WhatSon internal C++ modules.
- The view does not read, parse, mutate, or persist `.wsresource` metadata.
- The view does not create a generic resource editor contract; non-image resource handling remains outside this file.
- The selected resource entry must come from `ResourcesListModel.currentResourceEntry`.
- File path normalization is limited to turning an already-resolved absolute local path into a `file://` image source.

## 한국어

- 이 파일은 리소스 하이어라키에서 이미지 리소스를 선택했을 때 콘텐츠 영역에 표시되는 이미지 viewer다.
- 입력은 `ResourcesListModel.currentResourceEntry`에서 온 선택 리소스 entry뿐이다.
- `.wsresource` package 해석, resource metadata persistence, generic resource editor 정책은 맡지 않는다.
- `source`, `resolvedPath`, `resourcePath` 중 이미 모델이 제공한 경로만 이미지 source로 사용한다.
