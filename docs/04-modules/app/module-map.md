# app 모듈 맵 (Draft)

## 디렉터리 책임

- `src/app/file/**`: 파일 포맷 파싱/저장/정규화
- `src/app/viewmodel/**`: QML 바인딩용 상태/행동 계층
- `src/app/qml/**`: LVRS 기반 UI 조립 계층
- `src/app/runtime/**`: 스케줄러/병렬 로딩/런타임 유틸
- `src/app/permissions/**`: 권한 브리지

## 핵심 하위 시스템

1. Hierarchy 도메인
    - Library/Projects/Bookmarks/Tags/Resources/Progress/Event/Preset
    - 각 도메인별 Store/Parser/Creator + ViewModel/Model 구조
2. Editor 도메인
    - 본문 선택/논리라인/거터마커/세션 브리지를 분리
3. Sidebar 도메인
    - 도메인 선택 인덱스, 활성 ViewModel, 노트리스트 모델을 단일 ViewModel에서 해석
4. Runtime IO/Loader
    - 병렬 스냅샷 생성과 적용 경계 유지

## 상태

- Implemented: SRP 기반 분해가 테스트 계약으로 고정됨
- Partial: 일부 보조 레이어의 퍼블릭 API 문서화 미흡
