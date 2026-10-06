# `CMakePresets.json`

<a id="responsibility"></a>

## 책임
- CLion이 `.idea` 상태에 의존하지 않고 가져올 수 있는 저장소 소유 구성 프로필을 제공합니다.
- 프로젝트 빌드 정책과 일치하도록 `build/`에 고정된 기본 로컬 바이너리 디렉터리를 유지합니다.
- CMake 빌드 사전 설정을 통해 유지 관리되는 빌드 게이트를 노출하므로 IDE 및 셸 워크플로는 동일한 대상 이름을 사용합니다.

<a id="presets"></a>

## 사전 설정
- `macos-clion`: `Unix Makefiles` 생성기와 `${sourceDir}/build`를 `binaryDir`로 사용하여 macOS 개발 트리를 구성합니다.
- `whatson-build-regression`: `macos-clion` 구성 사전 설정에서 유지 관리되는 `whatson_build_regression` 대상을 빌드합니다.
- `whatson-regression`: `macos-clion` 구성 사전 설정에서 유지 관리되는 `whatson_regression` 대상을 빌드합니다.

<a id="invariants"></a>

## 불변성
- `build/`에서 기본 사전 설정을 리디렉션하지 마십시오.
- CLion 구성 문제를 해결하기 위해 `.idea` 프로필 상태를 커밋하지 마십시오; 대신 이 사전 설정 파일에 재현 가능한 계약을 유지하십시오.
- 루트 `CMakeLists.txt`가 이 프리셋에 사용자 전용 Qt 패치 버전을 고정하는 대신 `QT_ROOT_PATH`와 `/Volumes/Storage/Qt/6.*`를 통해 로컬 Qt 설치를 발견하도록 하십시오.

<a id="verification-notes"></a>

## 확인 메모
- 사전 설정 이름이나 스키마를 편집한 후 `cmake --list-presets`를 실행합니다.
- 캐시 변수 구성을 변경한 후 `cmake --preset macos-clion`를 실행합니다.
- 빌드 사전 설정 변경 후 `cmake --build --preset whatson-build-regression`를 실행합니다.
