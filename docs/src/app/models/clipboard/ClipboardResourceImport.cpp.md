# `src/app/models/clipboard/ClipboardResourceImport.cpp`

<a id="responsibility"></a>

## 책임

파일 형식 캡처가 완료된 후 클립보드 가져오기 설명자 구성을 구현합니다.

<a id="notes"></a>

## 메모

- MIME 및 파일 접미사 감지를 `FiletypeCapture`에 위임합니다.
- `WhatSonResourcePackageSupport`에서 타입 및 버킷 추론을 적용하고, 파일 형식이 알려진 후에도 클립보드 가져오기를 리소스 계층의 나머지와 정렬합니다.
- 패키지 생성 전에 메모리 내 페이로드가 구체화될 수 있도록 기본 클립보드 파일 이름을 제공합니다.

## 한국어

- clipboard 후보의 파일 형식 확인은 `FiletypeCapture`에 맡긴다.
- 이 파일은 확인된 format을 앱이 지원하는 resource type/bucket으로 연결해 import descriptor를 만든다.
- `.mp3`, `.m4a`, `.flac` 같은 음악 파일 확장자도 별도 `music` type으로 나누지 않고 canonical
  `audio`/`Audio`로 정규화한다.
