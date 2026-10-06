# src/app/cmake/resources/CMakeLists.txt

<a id="purpose"></a>

## 목적

이 샤드는 `WhatSon` 실행 가능 대상이 존재한 후 앱 소유 시각적 리소스를 등록합니다.

<a id="macos-bundle-icon"></a>

## macOS 번들 아이콘

- `resources/icons/app/desktop/AppIcon.png`는 인앱 QML 및 `QWindow`에서 사용되는 Qt qrc 아이콘으로 유지됩니다.
- `resources/icons/app/desktop/AppIcon.icns`는 macOS 애플리케이션 번들 아이콘입니다. `MACOSX_PACKAGE_LOCATION "Resources"`와 함께 소스로 `WhatSon` 실행 파일에 추가됩니다.
- `platform/Apple/Info.plist`는 `CFBundleIconFile`를 통해 동일한 파일에 이름을 지정하고, `src/app/cmake/runtime/CMakeLists.txt`는 `MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"`를 설정합니다.
- 샤드도 `WHATSON_MACOS_POST_BUILD_BUNDLE_ICON_FILE`를 내보내므로, 소유 앱 CMake 파일이 링크 후 아이콘을 `WhatSon.app/Contents/Resources/AppIcon.icns`에 복사할 수 있습니다. 이것은 선언적 번들 소스 사본을 구현하지 않는 제너레이터에서도 아이콘을 표시하도록 유지합니다.

앱 아이콘은 macOS `.app` 번들 경로에 있어야 합니다. 이 책임을 Qt qrc 자원으로 옮기지 마십시오.

<a id="other-platform-icons"></a>

## 다른 플랫폼 아이콘

  리소스 단계의 자산 카탈로그.
- Windows는 데스크탑 `.ico`를 가리키는 생성된 `.rc` 파일을 내보냅니다.
