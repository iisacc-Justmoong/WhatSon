# `src/app/models/sensor/UnusedNoteSensorSupport.hpp`

<a id="responsibility"></a>

## 책임

고정 기간 미사용 메모 센서 개체에서 사용하는 공유 RAW 메모 비활성 스캔을 선언합니다.

<a id="public-contract"></a>

## 공공 계약

- `collectUnusedNoteEntries(...)`는 압축 해제된 허브를 워킹하고, `.wsnhead`를 파싱하며, `WeeklyUnusedNote`와 `MonthlyUnusedNote`가 소비한 노트‐entry 페이로드를 반환합니다.
- `noteIdsFromEntries(...)`는 필터링된 식별자만 필요한 UI 호출자를 위해 해당 엔트리 페이로드에서 안정적인 `noteId` 목록을 추출합니다.
