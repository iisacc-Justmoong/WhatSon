# `src/app/models/file/viewer/ImageFormatCompatibilityLayer.cpp`

<a id="responsibility"></a>

## 책임
비트맵 미리보기에 대한 형식 정규화 및 런타임 호환성 검사를 구현합니다.

<a id="key-behavior"></a>

## 주요 행동
- 다음에서 형식 프로브를 허용합니다.
  - 직접 확장 값(`.PNG`, `jpeg`)
  - MIME 값(`image/jpeg`, `image/webp`)
  - 로컬 경로 및 URL 문자열(`/tmp/photo.png`, `file:///tmp/photo.png`)
- 알려진 별칭을 정규화합니다(`.jpeg` -> `.jpg`, `.tif` -> `.tiff`).
- `QImageReader::supportedImageFormats()`에서 호환성 세트를 빌드하고 이를 앱 내 비트맵 렌더링 가능성을 위한 단일 기준로 사용합니다.
- 미리보기를 앱 내에서 렌더링할 수 없는 경우 결정론적 비호환성 메시지를 반환합니다.

