# `src/app/qml/view/contents/ImageEditor.qml`

<a id="role"></a>

## 역할

`ImageEditor.qml`는 활성 목록 모델이 유형이나 형식이 이미지 지원되는 리소스 항목을 노출할 때 `ContentViewLayout.qml`에 의해 마운트되는 이미지 리소스 뷰어입니다.

프리젠테이션 전용 LVRS 내용 보기입니다. 선택한 리소스 항목 맵을 사용하고 `source`, `resolvedPath` 또는 `resourcePath`에서 QML 이미지 URL를 확인하고 `Image.PreserveAspectFit`를 사용하여 이미지를 렌더링합니다.

<a id="boundary"></a>

## 경계

- 보기는 WhatSon 내부 C++ 모듈을 가져오지 않습니다.
- 보기는 `.wsresource` 메타데이터를 읽거나 구문 분석하거나 변경하거나 유지하지 않습니다.
- 보기는 일반 리소스 편집기 계약을 생성하지 않습니다. 이미지가 아닌 리소스 처리는 이 파일 외부에 유지됩니다.
- 선택한 리소스 항목은 `ResourcesListModel.currentResourceEntry`에서 나와야 합니다.
- 파일 경로 정규화는 이미 확인된 절대 로컬 경로를 `file://` 이미지 소스로 변환하는 것으로 제한됩니다.

## 한국어

- 이 파일은 리소스 하이어라키에서 이미지 리소스를 선택했을 때 콘텐츠 영역에 표시되는 이미지 viewer다.
- 입력은 `ResourcesListModel.currentResourceEntry`에서 온 선택 리소스 entry뿐이다.
- `.wsresource` package 해석, resource metadata persistence, generic resource editor 정책은 맡지 않는다.
- `source`, `resolvedPath`, `resourcePath` 중 이미 모델이 제공한 경로만 이미지 source로 사용한다.
