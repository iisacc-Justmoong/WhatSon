# 도메인별 구현 현황 (Draft)

본 문서는 현재 코드와 기존 아키텍처 문서를 기준으로 정리한 진행중 상태 문서이다. 완결본이 아니며, 구현 변경 시 즉시 갱신되어야 한다.

## 공통 기준

- Implemented: 코드/문서에서 동작이 확인된 항목
- Partial: 계약/구조는 있으나 일부 경로 미완료
- Planned: 계획 또는 TODO 수준

## 1) Library 도메인

상태: Implemented (핵심), Partial (일부 고도화)

구현 확인 항목:

- `All Library`, `Draft`, `Today` 시스템 버킷 운용
- `.wsnindex` + `.wsnhead` + `.wsnbody` 기반 노트 인덱싱/메타 구성
- 본문 파싱(plain text), 이미지 리소스 썸네일 추출, 노트 카드 역할 제공
- 폴더 drag/drop 재구성(before/after/child/root) 및 `Folders.wsfolders` 즉시 반영
- 폴더 이동 시 관련 노트 헤더 `<folders>` 경로 재기록
- 노트 생성(`createEmptyNote`) 및 삭제(`WhatSonHubNoteDeletionService`) 파이프라인
- 고아 인덱스 정리(`WhatSonLibraryIndexIntegrityValidator`) 반영

부분 구현/보완:

- 대규모 데이터셋 성능 지표 문서화 미흡
- 동기화 충돌(conflict) 자동 병합 정책은 운영 기준 미완료

## 2) Projects 도메인

상태: Implemented (기본 계층 운용)

구현 확인 항목:

- `ProjectsHierarchyViewModel` 기반 계층 로딩/표시
- 폴더 drag/drop 이동 및 `Folders.wsfolders` 영속화
- 도메인별 Store/Parser/Creator 패턴 유지

부분 구현/보완:

- 프로젝트 고유 워크플로(상태/칸반/의존관계 등) 문서화 부재

## 3) Bookmarks 도메인

상태: Implemented (도메인 특화 모델)

구현 확인 항목:

- 북마크 상태/색상 파싱(`.wsnhead <bookmarks>`)
- 9색 기준 팔레트 기반 북마크 계층 운용
- 북마크 전용 NoteListModel 분리(`BookmarksNoteListModel`)
- Library 노트 삭제 신호 수신 시 북마크 투영 상태 동기화

부분 구현/보완:

- 북마크 고급 필터/정렬 정책 문서화 미흡

## 4) Tags 도메인

상태: Implemented (핵심), Partial (운영 고도화)

구현 확인 항목:

- `Tags.wstags` 파싱(트리/플랫 변형 수용)
- 허브 런타임 태그 depth 상태 구성(`WhatSonHubTagsStateStore`/DepthProvider)
- 태그 계층 ViewModel 연동 및 사이드바 표시

부분 구현/보완:

- 태그 리네임/병합 시 전역 참조 정합성 정책 명문화 필요

## 5) Resources 도메인

상태: Implemented (기본 계층/파일 연계)

구현 확인 항목:

- `Resources.wsresources` 기반 계층 파싱/스토어/생성
- 노트 본문 리소스 엔트리와 카드 썸네일 노출 연계

부분 구현/보완:

- 미디어 처리 파이프라인(인코딩/미리보기 정책) 문서화 필요

## 6) Progress 도메인

상태: Implemented (고정 상태 모델)

구현 확인 항목:

- `Progress.wsprogress` 기반 계층 로딩
- Progress 계층은 런타임에서 불변 라벨 중심 운용
- rename/create/delete 비활성 정책 적용

부분 구현/보완:

- 사용자 정의 progress taxonomy 확장 경로 미정

## 7) Event 도메인

상태: Implemented (기본 계층 운용)

구현 확인 항목:

- `Event.wsevent` 파싱/스토어/생성 패턴
- 이벤트 계층 ViewModel 연동

부분 구현/보완:

- 캘린더/리마인더와의 고급 동기화 정책은 문서화 미흡

## 8) Preset 도메인

상태: Implemented (기본 계층 운용)

구현 확인 항목:

- `Preset.wspreset` 파싱/스토어/생성
- 프리셋 계층 ViewModel 연동

부분 구현/보완:

- 프리셋 버저닝/상속 전략 미문서화

## 9) Hub Runtime 도메인(횡단)

상태: Implemented (핵심 런타임)

구현 확인 항목:

- `.wshub` 탐색 및 도메인 병렬 로딩
- `WhatSonHubRuntimeStore`로 hub/stat/placement/tags 상태 통합
- 도메인 실패 분리(일부 실패가 전체 중단으로 확산되지 않음)

부분 구현/보완:

- 프로파일별 충돌해결 운영 규칙 문서화 미흡

## 10) IO Runtime 도메인(횡단)

상태: Implemented

구현 확인 항목:

- 이벤트 큐 기반 IO 처리(`io.ensureDir`, `io.writeUtf8`, `io.appendUtf8`, `io.readUtf8`, `io.removeFile`)
- 시스템 IO 게이트웨이/런타임 컨트롤러 분리
- 구조화된 처리 결과 기록

부분 구현/보완:

- 장애 주입 테스트/복구 플레이북 문서화 필요

## 총평

현재 WhatSon 도메인 구조는 대부분 "구현 완료 + 운영 문서 보완" 단계에 있다. 즉 제품 기능의 중심축(계층·노트·편집·런타임 로딩)은 실코드 기준으로 동작하나, 충돌정책·성능기준·운영규약과 같은 상위 레벨
명세는 추가 정리가 필요하다.
