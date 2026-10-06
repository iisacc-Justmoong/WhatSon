# `src/app/models/detailPanel/DetailPanelState.cpp`

<a id="responsibility"></a>

## 책임
`DetailPanelState.cpp`는 내부 세부 패널 열거형 값을 외부에 표시되는 상태 ID에 매핑합니다.

<a id="current-state-id-contract"></a>

## 현재 상태 ID 계약
- `Properties` -> `properties`
- `FileStat` -> `fileStat`
- `Insert` -> `insert`
- `FileHistory` -> `fileHistory`
- `Layer` -> `layer`
- `Help` -> `help`

<a id="notes"></a>

## 메모
- 열거형 이름 자체는 내보낸 Figma 페이지 식별자를 따릅니다.
- QML는 이러한 ID를 사용하여 도구 모음 페이지와 콘텐츠 양식을 선택합니다.
