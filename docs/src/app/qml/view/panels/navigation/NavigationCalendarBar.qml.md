# `src/app/qml/view/panels/navigation/NavigationCalendarBar.qml`

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/view/panels/navigation/NavigationCalendarBar.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `NavigationCalendarBar.qml`
- 대략적인 줄 수: 54

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `LV.HStack`

<a id="object-ids"></a>

### 개체 ID
- `calendarBar`
- `taskButton`
- `dailyCalButton`
- `weeklyCalButton`
- `monthlyCalButton`
- `yearlyCalButton`

<a id="required-properties"></a>

### 필수 속성
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="signals"></a>

### 신호
- `viewHookRequested(string reason)`

<a id="interaction-contract"></a>

## 상호작용 계약
- 작업 아이콘(`taskButton`)은 복원된 가장 왼쪽 캘린더 바 어포던스입니다. 이전 체크리스트 글리프를 유지하지만 제거된 레거시 훅이 아니라 `open-task`를 방출합니다.
- 연도 아이콘(`yearlyCalButton`)은 `requestViewHook(...)`를 통해 `open-yearly-calendar`를 방출합니다.
- 이제 신호는 후크 이유 문자열을 전달하므로 상위 래퍼가 작업을 콘텐츠 오버레이로 라우팅할 수 있습니다.

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
