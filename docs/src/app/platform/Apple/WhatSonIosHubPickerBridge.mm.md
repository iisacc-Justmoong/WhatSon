# `src/app/platform/Apple/WhatSonIosHubPickerBridge.mm`

## Role
Implements the native iOS document-browser bridge used by onboarding to select a WhatSon hub from Files or third-party
providers such as Box.

## Picker Strategy
- Presents `UIDocumentBrowserViewController` from the active foreground window/controller instead of relying on
  `QtQuick.Dialogs`, which avoids the folder-picker/provider restrictions that disabled Box.
- Uses `initForOpeningContentTypes(...)` / `initForOpeningFilesWithContentTypes(...)` with a package-aware,
  provider-friendly type list:
  the exported `com.iisacc.whatson.hub` document type, `UTTypePackage`, and generic file content supertypes
  (`UTTypeContent`, `UTTypeItem`, `UTTypeData`).
- This keeps providers that understand package documents eligible for direct `.wshub` picks, while providers that only
  expose the package as a browsable directory can still return a nested file that onboarding remaps back to the
  enclosing hub root.
- Adds an app-owned `Open` browser action in the native navigation bar/menu so WhatSon still exposes an explicit
  confirmation affordance when the provider does not show the system `Open` button.
- That action now leaves `supportedContentTypes` at the browser default (`UTTypeItem.identifier`) instead of
  narrowing it to `.wshub`-specific types, which keeps provider-exposed nested files and folders eligible to activate
  the same `Open` action.

## Lifetime And Signals
- A dedicated Objective-C delegate forwards accept/cancel/failure callbacks back into Qt with queued invocations.
- The bridge tears down the picker/delegate references on accept, cancel, failure, and destruction so QML can safely
  reuse the same bridge object for repeated onboarding attempts.
