# `src/app/models/hierarchy/resources/ResourcesListModel.hpp`

<a id="responsibility"></a>

## 책임

리소스 계층 도메인에 대한 전용 오른쪽 패널 목록 모델을 선언합니다.

<a id="public-contract"></a>

## 공공 계약

- `ListBarLayout` 및 편집기 선택 연결에서 사용되는 공유 목록 브리지 계약을 공개합니다.
  - `itemCount`
  - `currentIndex`
  - `noteBacked`
  - `currentNoteId`
  - `currentBodyText`
  - `currentResourceEntry`
  - `searchText`
- `noteBacked` 는 영구적으로 `false` 입니다. 리소스 목록은 여전히 일반적인 목록 대리자를 위해 노트와 같은 id/본문 속성을 재사용하지만, 이러한 id 는 노트 지속성, 노트 헤더 또는 선택된 노트 본문 로더에 의해 실제 노트 패키지 id 로 취급되어서는 안 됩니다.
- 노트 카드 호환 역할( `noteId`, `primaryText`, `image`, `imageSource`, `displayDate`, `folders`, `tags`)을 제공하여 기존 리스트 델리인 UI를 `LibraryNoteListModel`를 사용하지 않고도 렌더링할 수 있도록 합니다.
- 리소스 인식 렌더링 경로를 위해 리소스별 역할( `type`, `format`, `resourcePath`, `resolvedPath`, `source`, `renderMode`, `displayName`, `previewText`)을 추가합니다.
- `currentResourceEntry`는 현재 선택된 리소스 페이로드를 맵으로 노출하므로, 전용 파일 뷰어가 노트 본문을 재파싱하지 않고도 목록 선택에서 렌더링할 수 있습니다.

<a id="intent"></a>

## 의도

이 모델은 의도적으로 일반 라이브러리 노트 목록 모델에서 리소스 목록 동작을 분리하므로 리소스 도메인 변경으로 인해 라이브러리 도메인 목록 의미가 회귀되지 않습니다.
