# `src/app/models/clipboard/InAppClipboardManager.cpp`

<a id="responsibility"></a>

## 책임

QML `inAppClipboard` 컨텍스트 개체에 대한 시스템 클립보드 캡처, URL/파일 가져오기 오케스트레이션 및 리소스 패키지 가져오기를 구현합니다.

<a id="notes"></a>

## 메모

- 로컬 파일 URL, 지원되는 MIME 페이로드, text/html 페이로드, 이미지, 픽맵, 및 이미지 데이터 URL 을 `QClipboard` / `QMimeData` 에서 읽습니다.
- 플랫폼 이미지 MIME 페이로드와 이미지 데이터 URL을 일반 텍스트/html 리소스 캡처 전에 추출하므로, 스크린샷 붙여넣기가 텍스트/html로 건너뛰거나 저장되는 대신 이미지 `.wsresource`로 가져옵니다.
- 현재 시스템 클립보드에서 리소스 가용성 스냅샷을 새로 고치며, 오래된 앱 내 스냅샷을 신뢰하지 않습니다. 캡처가 실패하면 이전 인앱 리소스 스냅샷이 삭제되고 편집기 붙여넣기 경로는 네이티브 붙여넣기로 되돌아가야 합니다.
- 앱 내부 로컬 파일, 원시 바이트 및 텍스트를 리소스 페이로드로 허용합니다.
- 단일 현재 클립보드 리소스 스냅샷을 `InAppClipboardStore`에 위임합니다.
- 로컬 파일 또는 구체화된 클립보드 페이로드를 `.wsresources/<id>.wsresource`에 유지합니다.
- 기본 `clipboard-resource.*` 임시 파일 이름을 사용하는 클립보드 페이로드가 32자리의 알파벳 숫자 리소스 ID 와 일치하는 자산 파일 이름으로 저장되므로, 반복적인 스크린샷 붙여넣기가 중복 파일 이름 충돌을 일으키지 않습니다. 기본 임시 파일 이름은 나중에 무작위 ID 로 최종 패키지 및 자산 이름이 생성되기 때문에 중복 사전 검사 에서 제외됩니다.
- `Resources.wsresources`를 업데이트하고, 중복 가져오기 정책을 처리하고, 편집기 삽입 메타데이터를 반환합니다.
- `ClipboardResourcePackageImport.cpp`를 다시 도입해서는 안 됩니다. 패키지 가져오기 파이프라인은 이 개체의 일부입니다.

## 한국어

- 시스템 clipboard에서 앱이 리소스로 받아들일 수 있는 항목을 캡처한다.
- platform image MIME payload와 image data URL은 일반 text/html 리소스 캡처보다 먼저 이미지로 추출한다.
- paste 직전 refresh는 기존 snapshot이 남아 있어도 현재 OS clipboard를 다시 캡처한다. 캡처에 실패하면 이전
  snapshot을 보존하지 않고 비워 native paste fallback으로 돌려보낸다.
- 이미지 전용이 아니며, 앱 내부에서 전달하는 local file, raw bytes, text payload도 같은 경로로 처리한다.
- 현재 붙여넣기 후보 하나의 저장 상태는 `InAppClipboardStore`가 소유한다.
- `.wsresource` 패키지 생성, `Resources.wsresources` 갱신, 충돌 처리는 manager가 조율한다.
- 기본 임시 이름인 `clipboard-resource.*`로 materialize된 clipboard payload는 32자 영문대소숫자 resource id와
  같은 asset 파일명으로 저장해 반복 스크린샷 붙여넣기 간 이름 충돌을 만들지 않는다. 이 기본 임시 이름은
  실제 저장 이름이 아니므로 duplicate preflight에서도 기존 `clipboard-resource.*` asset과 충돌시키지 않는다.
