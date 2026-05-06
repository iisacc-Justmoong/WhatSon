# `src/app/models/editor/tags/ContentsResourceTagController.cpp`

## Responsibility

Implements RAW resource-tag insertion and tag-loss detection.

## Current Behavior

- Normalizes imported resource entries before tag generation.
- Builds structured-document mutation payloads for resource insertion.
- Counts canonical resource tags before and after mutation so resource insertion cannot silently drop existing tags.
