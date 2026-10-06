# `src/app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 생성자는 이제 `IWhatSonNoteHeaderSessionStore` 베이스를 초기화합니다.
- 로드 중은 여전히 활성 노트 컨텍스트에 대한 `.wsnhead` 구문 분석/캐시 수명 주기를 소유하고 있습니다.
- `assignFolderBinding(...)`는 메모리 내 헤더 스냅샷만 업데이트합니다; 메모 패키지를 삭제한 후에는 구체적인 메모 헤더 영속성이 비활성화됩니다.
- `assignTag(...)`는 이제 활성 헤더에 동일한 대소문자로 접힌 태그가 이미 포함되어 있지 않은 경우에만 정규화된 태그 값을 추가하고, 동일한 공유 캐시 항목을 통해 지속됩니다.
- 색인화된 폴더/태그 제거는 캐시된 헤더 스냅샷을 변경하고 파일 지속성 없이 캐시를 정리한 것으로 표시합니다.
