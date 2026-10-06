# `src/app/qml/view/panels/PanelEdgeSplitter.qml`

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/view/panels/PanelEdgeSplitter.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `PanelEdgeSplitter.qml`
- 대략적인 줄 수: 58

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `Rectangle`

<a id="object-ids"></a>

### 개체 ID
- `splitter`
- `splitterMouse`

<a id="required-properties"></a>

### 필수 속성
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="signals"></a>

### 신호
- `sizeDragRequested`

<a id="recent-updates"></a>

## 최근 업데이트
- 드래그 크기 방출 전에 로컬 함수 참조를 통해 선택적 클램프 콜백을 해결하는 `resolveClampedSize(candidateSize)` 헬퍼를 추가했습니다.
- 드래그 모션 경로가 이제 var 속성 인라인을 호출하는 대신 `splitter.resolveClampedSize(nextSize)`를 호출합니다.

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
