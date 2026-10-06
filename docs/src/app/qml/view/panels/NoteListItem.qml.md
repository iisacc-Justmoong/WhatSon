# `src/app/qml/view/panels/NoteListItem.qml`

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/view/panels/NoteListItem.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `NoteListItem.qml`
- 대략적인 줄 수: 320

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `Item`

<a id="object-ids"></a>

### 개체 ID
- `noteListItem`
- `noteHoverHandler`
- `foldersRow`
- `tagsRow`

<a id="required-properties"></a>

### 필수 속성
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="signals"></a>

### 신호
- `viewHookRequested`

<a id="recent-updates"></a>

## 최근 업데이트
- 중첩된 메타데이터 행에 대해 바인딩된 대리인 ID 액세스를 적용하기 위해 `pragma ComponentBehavior: Bound`를 추가했습니다.
- 폴더/태그 메타데이터 `Repeater` 대리자는 이제 `required property var modelData`를 선언하고 `folderLabelRow.modelData` / `tagLabelRow.modelData`를 명시적으로 읽습니다.
- `primaryText` 는 메모 목록 모델에서 제공하는 표시 준비 메모 메타데이터입니다. 컴포넌트는 메모 소스 태그를 자체적으로 파싱하거나 제거하지 않아야 합니다.
- 리소스 행은 더 이상 이 구성 요소를 재사용하지 않으며, `ListBarLayout.qml`는 바인드 모델이 `currentResourceEntry`를 노출할 때 전용 `ResourceListItem.qml`를 마운트합니다.
- 카드 패딩, 미리보기 크기, 메타데이터 간격 및 암시적 행 기하학이 이제 고정된 픽셀·색상 리터럴 대신 지정된 `LV.Theme` 간격, 아이콘, 타이포그래피 및 패널 토큰에서 해결되므로, 메모 카드는 LVRS를 따릅니다.
- 북마크 캔버스 글리프가 이제 라이브 프레임 크기에서 포인트를 파생하므로, 북마크 마크는 LVRS 크기의 아이콘 프레임에 따라 스케일링되며, `16px` 경로에 고정된 상태를 유지하지 않습니다.
- 이제 이미지 자리 표시자는 이전의 원시 회색 채우기 대신 LVRS `strokeSoft` 토큰을 사용합니다.

<a id="intended-detailed-sections"></a>

## 의도된 세부 섹션
- 책임과 비즈니스 역할
- 소유권 및 수명주기
- 공개 API 또는 외부에서 관찰된 바인딩
- 협력자 및 의존성 방향
- 데이터 흐름 및 상태 전환
- 오류 처리 및 복구 경로
- 관련된 경우 스레딩, 스케줄링 또는 UI 선호도 제약 조건
- 확장점, 불변성 및 알려진 복잡성 핫스팟
- 테스트 적용 범위 및 검증 누락

<a id="authoring-notes-for-next-pass"></a>

## 다음 패스에 대한 작성 노트
- 이 스캐폴드를 교체하기 전에 실제 구현과 인접한 헤더를 읽어보세요.
- 해당하는 경우 구체적인 신호, 슬롯, 호출 가능 항목, 지속성 부작용 및 LVRS/QML 바인딩을 문서화합니다.
- 세부 단계가 시작되면 이 파일을 동일한 디렉터리에 있는 피어 모듈과 상호 연결하세요.
