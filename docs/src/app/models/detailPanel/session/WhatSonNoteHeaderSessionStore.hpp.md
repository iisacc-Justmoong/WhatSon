# `src/app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.hpp`

<a id="role"></a>

## 역할
`WhatSonNoteHeaderSessionStore`는 구체적인 노트 헤더 세션 캐시 및 지속성 서비스입니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- `IWhatSonNoteHeaderSessionStore`를 구현합니다.
- 세션 계약만 형제 컨트롤러에 노출하면서 내부 캐시 저장소를 비공개로 유지합니다.
- 공개 변이 표면은 이제 `assignFolderBinding(...)`와 `assignTag(...)`를 모두 포괄하므로, 상세 패널 추가 흐름은 원시 노트 헤더 영속성 코드에 접근하지 않고 동일한 파일 기반 세션 캐시를 공유할 수 있습니다.
