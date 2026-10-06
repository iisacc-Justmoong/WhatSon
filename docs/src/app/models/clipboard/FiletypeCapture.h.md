# `src/app/models/clipboard/FiletypeCapture.h`

<a id="responsibility"></a>

## 책임

페이로드가 `ClipboardResourceImport`가 되기 전에 사용되는 클립보드 파일 형식 캡처 도우미를 선언합니다.

<a id="contract"></a>

## 계약

- MIME 유형 문자열을 정규화합니다.
- MIME 유형을 리소스 파일 형식에 매핑합니다.
- 플랫폼 이미지 페이로드 MIME 이름(예: `application/x-qt-image` 및 `com.apple.tiff`)을 감지하고, 페이로드가 클립보드 리소스로 구체화되기 전에 이를 처리합니다.
- 파일 이름을 먼저 확인하고 MIME 유형을 두 번째로 확인하여 파일 형식을 확인합니다.
- 메모리 내 클립보드 페이로드에 대한 기본 파일 이름을 생성합니다.

## 한국어

- clipboard payload의 파일 형식 확인 책임은 이 객체에 있다.
- macOS/Qt 스크린샷 clipboard가 쓰는 `application/x-qt-image`, `com.apple.tiff` 같은 platform image MIME 판별도
  이 객체의 책임이다.
- `ClipboardResourceImport`는 이 객체가 산출한 format을 받아 resource descriptor를 구성한다.
