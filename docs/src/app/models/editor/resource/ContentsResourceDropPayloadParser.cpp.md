# `src/app/models/editor/resource/ContentsResourceDropPayloadParser.cpp`

## Responsibility

Implements drop payload normalization for resource imports.

## Current Behavior

- Reads native URL-list style payloads when available.
- Falls back to text/URI-list string payloads.
- Returns normalized URL strings for the import controller.
