# `src/app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp`

## Responsibility

Defines the shared C++ bridge base for editor display ViewModels.

## Contract

- Exposes a `controller` property with `controllerChanged`.
- Provides slot-based controller assignment.
- Keeps invocation helpers protected so concrete ViewModels publish only their single-purpose command surface.
