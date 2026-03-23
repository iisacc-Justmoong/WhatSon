# 빌드/설치 가이드 (Draft)

## 기본 빌드

```bash
cmake -S . -B build
cmake --build build -j
```

## LVRS 경로 지정 예시

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/.local/LVRS
```

## 루트 타깃(현재 확인)

- `whatson_build_all`
- `whatson_run_app`
- `whatson_export_binaries`
- `whatson_package`

## 개발 품질 타깃

- `whatson_qmllint`
- `whatson_qmlformat_check`
- `whatson_qmlformat_fix`
- `whatson_clang_tidy`
- `whatson_dev_checks`

## 설치 관련 메모

- LVRS 설치는 플랫폼 부트스트랩(wasm/macos/ios/android)에 따라 시간이 길어질 수 있다.
- 운영 관점에서는 실사용 플랫폼만 지정한 설치가 더 안정적이다.

## Mobile Build Script Notes

- The Android packaging stage must stage a fallback `res/drawable/app_icon.png` before density-specific icons so Qt Android generation can complete on a fresh build tree.
- Mobile artifact cleanup retries transient macOS `Directory not empty` failures before the script aborts.
- Desktop build trees now place the runnable host artifact at the build-directory root: `build/WhatSon.app`, `build/WhatSon.exe`, or `build/WhatSon`.
- `scripts/build_host.py` and `scripts/build_all.py` keep the packaged host artifact at `build/host-auto/WhatSon.app`, `build/host-auto/WhatSon.exe`, or `build/host-auto/WhatSon` even when a later aggregate host subtarget fails.
- `scripts/build_android.py` stages the installable APK at `build/android-auto/WhatSon.apk`.
- `scripts/build_ios.py` stages the device-ready app bundle at `build/ios-xcode-artifact/WhatSon.app`.
- The host `WhatSon` target no longer depends on iOS Xcode project export, so stale simulator-side CMake cache entries
  under `build/ios-xcode-artifact` cannot break unrelated macOS app builds.
- The explicit root `whatson_export_xcodeproj` target now clears only the nested iOS cache/state files before
  reconfiguring, instead of deleting the whole artifact directory.

## TODO

- CI 환경별 권장 configure preset 문서화
- 플랫폼별 실패 사례(특히 iOS 툴체인 경고) 대응 절차 추가
