# `src/extension/trial/WhatSonTrialSecureStore.hpp`

<a id="role"></a>

## 역할
재설치 방지 로컬 상태를 유지하기 위해 평가판 모듈에서 사용하는 선택적 OS 보안 저장소 브리지를 선언합니다.

<a id="public-api"></a>

## 공개 API
- `WhatSonTrialSecureStoreStatus`: 보안 저장소 읽기 또는 쓰기의 성공, 누락, 사용할 수 없음 또는 실패 여부를 설명합니다.
- `WhatSonTrialSecureStoreReadResult` / `WhatSonTrialSecureStoreWriteResult`: 저장 작업을 위한 작은 결과 구조체입니다.
- `WhatSonTrialSecureStoreBackend`: 플랫폼별 어댑터로 교체할 수 있는 백엔드 인터페이스.
- `WhatSonTrialSecureStore`: 하나의 서비스 이름 아래 비밀 항목의 범위를 지정하는 값 유형 래퍼입니다.

<a id="notes"></a>

## 메모
- 평가판 모듈은 클라이언트 ID 및 등록 무결성 비밀을 위해 보안 저장소를 사용합니다.
- 레거시 보안 저장소 설치 날짜는 계속해서 이전될 수 있지만 새 설치 날짜 쓰기는 더 이상 미러링된 보안 저장소 복사본을 유지하지 않습니다.
- 비프로덕션 흐름은 실제 호스트 보안 저장소를 건드리는 대신 메모리 내 백엔드를 주입할 수 있습니다.
