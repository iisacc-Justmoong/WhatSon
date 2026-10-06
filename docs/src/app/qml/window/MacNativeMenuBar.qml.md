# `src/app/qml/window/MacNativeMenuBar.qml`

<a id="role"></a>

## 역할

이 파일은 macOS 전역 메뉴 막대를 정의한다. 현재 사용자에게 보이는 2개의 경로를 노출한다.

- `File > Import File...`는 네이티브 멀티 파일 픽커를 열고 선택된 로컬 파일을 현재 허브 리소스 가져오기 흐름으로 전달합니다.
- `Window > Onboarding` 루트 창 온보딩 도우미를 다시 엽니다.

<a id="required-properties"></a>

## 필수 속성

- `hostWindow`는 `Window` 메뉴 작업에서 사용되는 루트 창 객체입니다.
- `inAppClipboard` 현재 허브에 대해 선택된 로컬 파일 URL을 수락하는 클립보드/임포트 백엔드.

<a id="import-flow"></a>

## 가져오기 흐름

1. 사용자가 `Import File...`를 트리거합니다.
2. `FileDialog.OpenFiles`는 하나 이상의 로컬 파일을 수집합니다.
3. 수락하면 메뉴 바가 `selectedImportUrls()`를 통해 피커 출력을 정규화하고, 이후 `inAppClipboard.inspectImportConflictForUrls(selectedFiles)`에게 현재 `*.wsresources` 스토어에 이미 존재하는 들어오는 자산 이름이 있는지 묻습니다.
4. 중복이 존재하는 경우, 메뉴 바는 호스트 창 콘텐츠 표면에 장착된 `LV.Alert`를 열고 사용자가 `Overwrite`, `Keep Both` 또는 `Cancel Import`를 선택하도록 합니다.
5. 사용자가 정책을 선택한 후, 메뉴 막대가 `inAppClipboard.importUrlsWithConflictPolicy(selectedFiles, policy)`를 호출합니다.
6. 가져오기가 실패하면 `lastError`를 읽고 모달 실패 대화 상자를 엽니다.

`selectedImportUrls()`는 의도적으로 `selectedFiles` 및 `selectedFile` 경로를 모두 병합하므로 가져오기는 macOS 네이티브 대화 상자 백엔드의 선택기 페이로드 모양 차이에 걸쳐 작동합니다.

성공적인 가져오기는 런타임 리소스 다시 로드를 사용하여 UI를 즉시 새로 고치므로 이 메뉴는 별도의 성공 알림을 내보내지 않습니다.
