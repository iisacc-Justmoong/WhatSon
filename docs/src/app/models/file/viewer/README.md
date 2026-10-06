# `src/app/models/file/viewer`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/viewer`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `ImageFormatCompatibilityLayer.cpp`
- `ImageFormatCompatibilityLayer.hpp`

## Current Notes
- The former body-resource renderer and bitmap viewer bridges were removed with the retired editor rendering/resource
  editor surface.
- This directory now keeps only reusable image-format compatibility helpers that are still part of the file/resource
  domain.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/file/viewer``  ( `docs/src/app/models/file/viewer/README.md` )
- Location:  `docs/src/app/models/file/viewer`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
