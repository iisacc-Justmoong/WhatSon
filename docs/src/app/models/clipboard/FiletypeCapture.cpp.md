# `src/app/models/clipboard/FiletypeCapture.cpp`

<a id="responsibility"></a>

## 책임

클립보드 가져오기를 위해 MIME 및 파일 이름 기반 리소스 형식 감지를 구현합니다.

<a id="notes"></a>

## 메모

- MIME 형식 테이블을 `ClipboardResourceImport`에서 유지합니다.
- `WhatSonResourcePackageSupport` 형식 정규화를 사용하여 클립보드 붙여넣기와 허브 리소스 가져오기가 동일한 형식 어휘를 공유합니다.
- MIME 플랫폼 이미지 페이로드 macOS 와 Qt 클립보드 브릿지에서 사용하는 광범위한 `application/x-qt-image` 및 `com.apple.tiff` 이름을 인식합니다.
- 알 수 없는 이미지 MIME 제품군을 `.png`로 대체하고 일반 옥텟 스트림 페이로드를 `.bin`로 대체합니다.

## 한국어

- MIME type과 파일명 확장자에서 hub resource format을 산출한다.
- OS 스크린샷 clipboard처럼 정확한 resource MIME 표준명이 아니라 platform image payload 이름으로 들어오는 경우도
  이미지 후보로 판별한다.
- `ClipboardResourceImport`는 이 파일의 결과를 사용하지만, MIME table 자체를 소유하지 않는다.
