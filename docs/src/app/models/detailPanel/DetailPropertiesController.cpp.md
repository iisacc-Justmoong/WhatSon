# `src/app/models/detailPanel/DetailPropertiesController.cpp`

<a id="responsibility"></a>

## 책임
- 메모 헤더 메타데이터를 세부 정보 패널의 속성 섹션에 투영합니다.
- 하나의 경량 컨트롤러에 폴더/태그 항목 목록과 현재 프로젝트, 북마크 및 진행률 값을 유지합니다.

<a id="folder-presentation-rules"></a>

## 폴더 표시 규칙
- 폴더 항목은 컴팩트한 칩 스타일 프레젠테이션을 위해 `leafFolderName(...)`를 선호합니다.
- 지속된 폴더 경로에 논리 세그먼트가 하나만 있는 경우, 대체 경로는 이제 원시 저장값 대신 `displayFolderPath(...)`를 사용합니다.
- 이는 하나의 폴더 라벨에 문자 그대로 `/`가 포함된 경우, `\/`와 같은 이스케이프된 영속성 마커가 상세 패널로 새어나오는 것을 방지합니다.

<a id="tests"></a>

## 테스트
- 유지 관리되는 C++ 회귀 제품군은 이 보기가 의존하는 공유 폴더 경로 이스케이프 의미 체계를 잠급니다.
