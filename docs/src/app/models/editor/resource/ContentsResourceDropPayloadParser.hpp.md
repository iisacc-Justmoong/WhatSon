# `src/app/models/editor/resource/ContentsResourceDropPayloadParser.hpp`

## Responsibility

Declares the C++ resource drop payload parser used by the editor import controller.

## Contract

- Normalizes native drop payload objects into file URL lists.
- Does not depend on editor session, parser, or persistence state.
- Replaces the removed QML payload parser.
