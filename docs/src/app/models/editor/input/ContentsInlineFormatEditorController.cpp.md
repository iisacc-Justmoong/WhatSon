# `src/app/models/editor/input/ContentsInlineFormatEditorController.cpp`

## Responsibility

Implements the C++ bridge for the active inline note editor controller.

## Current Behavior

- Loads its helper from `qrc:/qt/qml/WhatSon/App/view/contents/editor/ContentsInlineFormatEditorController.qml`.
- Synchronizes the `control` and `textInput` object handles into that helper.
- Forwards native focus, selection, cursor, deferred text-sync, and committed edit dispatch requests.

## Boundary

This file is intentionally narrow. Removed structured block controllers must not be reintroduced here; block parsing,
rendering, and minimap projection belong to their dedicated model layers.
