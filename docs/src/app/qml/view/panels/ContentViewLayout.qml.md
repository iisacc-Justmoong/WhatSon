# `src/app/qml/view/panels/ContentViewLayout.qml`

`ContentViewLayout.qml`는 중앙 작업 공간 콘텐츠 표면을 구성합니다.

<a id="current-contract"></a>

## 현재 계약

- `src/app/qml/view/contents`에서 `ImageEditor.qml`만 마운트합니다.
- `noteListModel.currentResourceEntry`를 통해 선택된 이미지 리소스는 `ImageEditor.qml`로 라우팅됩니다.
- 이미지가 아닌 경로나 메모 경로에는 레이아웃 연속성을 위해 빈 콘텐츠 자리 표시자가 표시됩니다.
- 캘린더 오버레이는 일시적으로 편집기 표면을 대체하고 라이브러리 계층 컨트롤러를 통해 노트 열기 요청을 다시 라우팅합니다.

<a id="deleted-document-model-boundary"></a>

## 삭제된 문서 모델 경계

활성 노트 문서 모델이 제거되었습니다. 이 레이아웃은 더 이상 편집기 문서 세션 개체, 편집기 붙여넣기 브리지, 네이티브 키 필터, RAW 푸시/풀 후크, 인라인 서식 바로가기 또는 선택 컨텍스트 메뉴를 수신하거나 전달하지 않습니다.

이 파일의 QML는 활성 노트 본문 파일을 마운트하거나, 소스 태그를 변경하거나, 문서 세션 호환성 래퍼를 다시 도입해서는 안 됩니다.
