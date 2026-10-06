# `src/app/models/clipboard/ClipboardResourceImport.h`

<a id="responsibility"></a>

## 책임

가져올 수 있는 클립보드 리소스 하나를 설명하는 데 사용되는 간단한 값 개체를 선언합니다.

<a id="contract"></a>

## 계약

- 소스 파일 이름, 선택적 로컬 파일 경로, MIME 유형, 정규화된 리소스 형식, 유형, 버킷 및 선택적 메모리 내 페이로드를 저장합니다.
- `InAppClipboardManager`를 통해 제공되는 파일 이름, 로컬 파일, 이미지, 원시 바이트 및 텍스트 기반 페이로드에서 가져오기를 빌드할 수 있는 도우미를 제공합니다.
- `FiletypeCapture`의 파일 형식 결정을 소비한 후, 리소스 패키지 분류 체계를 사용하여 클립보드 이미지, PDF, 텍스트/ HTML 문서, 오디오 파일, 3D 모델, 아카이브 및 기타 지원되는 형식이 가져온 파일과 동일한 유형/버킷 레이블로 변환됩니다.

## 한국어

- clipboard에서 얻은 단일 리소스를 설명하는 값 객체다.
- 파일명과 MIME type 확인은 `FiletypeCapture`가 맡고, 이 값 객체는 그 결과를 resource format/type/bucket
  목록에 맞춰 보관한다.
- 이미지뿐 아니라 문서, 텍스트/HTML, 오디오, 비디오, 3D 모델, 압축 파일 payload도 같은 값 객체로 표현한다.
- payload 자체를 본문에 넣지 않고, 이후 import 단계가 `.wsresource` package로 materialize할 수 있는 정보만 담는다.
