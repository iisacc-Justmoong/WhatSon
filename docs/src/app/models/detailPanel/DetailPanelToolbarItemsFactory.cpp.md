# `src/app/models/detailPanel/DetailPanelToolbarItemsFactory.cpp`

<a id="responsibility"></a>

## 책임
이 팩토리는 `DetailPanelController`에서 사용하는 표준 도구 모음 사양 목록을 작성합니다. 6개의 세부 패널 상태 버튼과 해당 아이콘 이름은 C++ 기준 원본입니다.

<a id="current-mapping"></a>

## 현재 매핑
- `Properties` -> `config`
- `FileStat` -> `chartBar`
- `Insert` -> `generaladd`
- `Layer` -> `toolwindowdependencies`
- `FileHistory` -> `toolWindowClock`
- `Help` -> `featureAnswer`

<a id="output-shape"></a>

## 출력 형태
생성된 각 도구 모음 항목은 다음을 포함하는 `QVariantMap`입니다.
- `iconName`
- `stateValue`
- `selected`

<a id="notes"></a>

## 메모
- 첫 번째 아이콘은 이전 프로젝트 구조 문자 모양에서 `config`로 변경되어 데스크탑 세부 도구 모음이 Figma `Properties` 버튼 계약과 일치합니다.
- QML는 검사 및 테스트를 위해 Figma 노드 ID와 의미 개체 이름으로 이러한 항목을 계속 강화합니다.
