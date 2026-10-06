# `src/app/models/file/viewer/ImageFormatCompatibilityLayer.hpp`

<a id="responsibility"></a>

## 책임
인앱 비트맵 뷰어에서 사용되는 이미지 형식 호환성 레이어를 선언합니다.

<a id="public-contract"></a>

## 공공 계약
- `normalizedBitmapFormat(...)`는 포맷 프로브(확장, MIME 문자열, 로컬 경로 또는 URL)를 정규화된 소문자 확장 토큰(예: `.jpg`, `.png`)으로 변환합니다.
- `isBitmapFormatCompatible(...)`는 현재 Qt 이미지 리더 런타임가 정규화된 형식을 지원하는지 확인합니다.
- `unsupportedBitmapFormatMessage(...)`는 지원되지 않거나 누락된 비트맵 형식 메타데이터에 대해 안정적인 사용자용 메시지를 생성합니다.

