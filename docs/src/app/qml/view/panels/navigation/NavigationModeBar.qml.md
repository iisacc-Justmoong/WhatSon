# `src/app/qml/view/panels/navigation/NavigationModeBar.qml`

<a id="lvrs-token-notes"></a>

## LVRS 토큰 노트
- 컴팩트 모드 간격은 `LV.Theme.gap8` / `LV.Theme.gapNone`를 사용합니다.

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/view/panels/navigation/NavigationModeBar.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `NavigationModeBar.qml`
- 대략적인 줄 수: 84

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `LV.HStack`

<a id="object-ids"></a>

### 개체 ID
- `modeBar`
- `modeCombo`
- `modeContextMenu`

<a id="required-properties"></a>

### 필수 속성
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="signals"></a>

### 신호
- `viewHookRequested`

<a id="recent-updates"></a>

## 최근 업데이트
- Context-menu `selectedIndex`는 이제 `modeBar.navigationModeController`를 통해 해결되어 중첩된 메뉴 바인딩을 루트 ID에 명시적으로 적용하도록 유지합니다.
- 모드 레이블 및 컨텍스트 메뉴 대체 경로는 바인드된 탐색 모드 컨트롤러가 아직 해결되지 않은 경우 이제 `View` ( `selectedIndex: 0` )로 기본 설정됩니다.
- 팝업/메뉴 메트릭은 이제 고정 리터럴 대신 `comboMenuYOffset` ( `LV.Theme.gap2` )와 `comboContextMenuWidth` ( `LV.Theme.buttonMinWidth + LV.Theme.gap24 + LV.Theme.gap8` )를 사용합니다.

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
