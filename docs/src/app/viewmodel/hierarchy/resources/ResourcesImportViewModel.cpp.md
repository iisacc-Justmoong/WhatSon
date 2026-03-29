# `src/app/viewmodel/hierarchy/resources/ResourcesImportViewModel.cpp`

## Responsibility

구현은 세 단계를 순서대로 끝낸다.

1. 드롭 URL에서 실제 local file만 추린다
2. 각 파일을 `.wsresource` 패키지로 current hub에 복사한다
3. `Resources.wsresources`를 다시 쓰고 resources runtime reload callback을 호출한다

## Import Semantics

- source file 이름은 패키지 내부 asset 이름으로 그대로 유지된다
- package id는 파일 base name을 slug화한 뒤 existing package와 충돌하지 않도록 suffix를 붙인다
- `type`/`bucket`/`format`은 `WhatSonResourcePackageSupport.hpp` 규칙을 그대로 사용한다

## Failure Rule

패키지 생성 단계나 `Resources.wsresources` 재기록 단계가 실패하면 그 turn에서 만든 패키지 디렉터리는 rollback된다.
반대로 저장은 끝났지만 runtime refresh callback이 실패한 경우에는 import 결과는 유지하고, ViewModel은 실패 신호를 올려 UI가 사용자에게 알려주도록 한다.
