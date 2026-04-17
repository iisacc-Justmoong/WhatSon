# `src/app/viewmodel/content/ContentsResourceTagTextGenerator.cpp`

## Responsibility
Bridges QML import flows onto the note-body resource tag generator without leaving RAW tag serialization in JavaScript.

## Runtime Notes
- The bridge is QML-instantiated through `WhatSonQmlInternalTypeRegistrar.cpp`.
- The same generator instance can normalize import payloads and emit final tag text, keeping drag/drop and clipboard
  insertions on one canonical C++ path.
