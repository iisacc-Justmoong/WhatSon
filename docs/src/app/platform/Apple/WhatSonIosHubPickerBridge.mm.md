# `src/app/platform/Apple/WhatSonIosHubPickerBridge.mm`

## Role
Implements the native iOS document-picker bridge used by onboarding to select a WhatSon hub from Files or third-party
providers such as Box.

## Picker Strategy
- Presents `UIDocumentPickerViewController` from the active foreground window/controller instead of relying on
  `QtQuick.Dialogs`, which avoids the folder-picker/provider restrictions that disabled Box.
- Uses `initForOpeningContentTypes(..., asCopy:NO)` with file-oriented provider-friendly content types
  (`UTTypeItem`, `UTTypeContent`, `UTTypeData`), so Box can keep the native `Open` affordance active while the
  onboarding controller still resolves the picked nested file back to the enclosing `.wshub` package.
- Leaves the native Files chrome intact, which preserves the platform `Open` affordance the onboarding flow needs.

## Lifetime And Signals
- A dedicated Objective-C delegate forwards accept/cancel/failure callbacks back into Qt with queued invocations.
- The bridge tears down the picker/delegate references on accept, cancel, failure, and destruction so QML can safely
  reuse the same bridge object for repeated onboarding attempts.
