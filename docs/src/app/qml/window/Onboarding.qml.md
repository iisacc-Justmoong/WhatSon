# `src/app/qml/window/Onboarding.qml`

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/window/Onboarding.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `Onboarding.qml`
- 대략적인 줄 수: 119

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `Window`

<a id="current-notes"></a>

## 현재 노트
  원시 `542/867/420/620/762/470px` 리터럴 대신 구성.
- 외부 투명 표면은 `LV.Theme.accentTransparent`를 사용합니다.
- 외부 창은 여전히 모든 시각적 크롬을 `OnboardingContent.qml`에 위임합니다; 이 파일은 창 크기 지정, 리젠터링, 모달리티 및 라우트‐투‐콘텐츠 배선만을 보유하고 있습니다.
- 일반적인 데스크톱 시작은 이제 다시 이 래퍼에 의존합니다: 영구 허브를 마운트할 수 없을 경우, `Main.qml`는 작업 공간 셸을 백그라운드에서 유지하고 이 별도의 모달 온보딩 창을 상단에 올립니다.
- `Onboarding.qml`도 명시적인 `whatson --onboarding-only` 셸을 유지하므로, 데스크톱 시작 복구와 전용 온보딩 전용 엔트리포인트가 동일한 창 소유자를 공유합니다.

<a id="object-ids"></a>

### 개체 ID
- `root`

<a id="required-properties"></a>

### 필수 속성
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="signals"></a>

### 신호
- `createFileRequested`
- `dismissed`
- `selectFileRequested`
- `viewHookRequested`

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
