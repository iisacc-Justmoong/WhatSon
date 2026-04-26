# `src/app/models/editor/tags/ContentsResourceTagTextGenerator.cpp`

## Responsibility
Implements canonical RAW `<resource ... />` tag text generation and publishes the last generated descriptor/tag text
for QML import coordinators.

## Boundary
- This file does not own resource package creation or bitmap/PDF rendering.
- It is the editor-facing adapter over the lower-level note-body resource tag generator.
