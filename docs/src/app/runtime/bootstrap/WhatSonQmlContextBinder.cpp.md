# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

`bindWorkspaceContextObjects(...)`는 작업공간 런타임 개체에 대한 LVRS QML 컨텍스트 바인딩 계획을 빌드합니다.

바인딩 목록은 이제 삭제된 활성 편집기 문서 세션·편집기 붙여넣기 브리지·네이티브 편집기 입력 명령 필터를 제외한다. 남은 바인딩은 계층 컨트롤러·상세 패널 컨트롤러·탐색 상태·클립보드 가져오기 상태·캘린더 컨트롤러·비동기 스케줄링·패널 컨트롤러 레지스트리를 포함한다.
