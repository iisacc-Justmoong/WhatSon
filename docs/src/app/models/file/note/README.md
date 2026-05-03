# `src/app/models/file/note`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/note`
- Child directories: 0
- Child files: 39

## Child Directories
- No child directories.

## Child Files
- `WhatSonBookmarkColorPalette.hpp`
- `ContentsNoteManagementCoordinator.cpp`
- `ContentsNoteManagementCoordinator.hpp`
- `WhatSonHubNoteCreationService.cpp`
- `WhatSonHubNoteCreationService.hpp`
- `WhatSonHubNoteDeletionService.cpp`
- `WhatSonHubNoteDeletionService.hpp`
- `WhatSonHubNoteFolderClearService.cpp`
- `WhatSonHubNoteFolderClearService.hpp`
- `WhatSonHubNoteMutationSupport.cpp`
- `WhatSonHubNoteMutationSupport.hpp`
- `WhatSonLocalNoteDocument.hpp`
- `WhatSonLocalNoteFileStore.cpp`
- `WhatSonLocalNoteFileStore.hpp`
- `WhatSonNoteBodyCreator.cpp`
- `WhatSonNoteBodyCreator.hpp`
- `WhatSonNoteBodyResourceTagGenerator.cpp`
- `WhatSonNoteBodyResourceTagGenerator.hpp`
- `WhatSonNoteBodyPersistence.cpp`
- `WhatSonNoteBodyPersistence.hpp`
- `WhatSonNoteBodyWebLinkSupport.cpp`
- `WhatSonNoteBodyWebLinkSupport.hpp`
- `WhatSonNoteBodySemanticTagSupport.cpp`
- `WhatSonNoteBodySemanticTagSupport.hpp`
- `WhatSonNoteCreator.cpp`
- `WhatSonNoteCreator.hpp`
- `WhatSonNoteFolderBindingRepository.cpp`
- `WhatSonNoteFolderBindingRepository.hpp`
- `WhatSonNoteFolderBindingService.cpp`
- `WhatSonNoteFolderBindingService.hpp`
- `WhatSonNoteFolderSemantics.hpp`
- `WhatSonNoteHeaderCreator.cpp`
- `WhatSonNoteHeaderCreator.hpp`
- `WhatSonIiXmlDocumentSupport.cpp`
- `WhatSonIiXmlDocumentSupport.hpp`
- `WhatSonNoteHeaderParser.cpp`
- `WhatSonNoteHeaderParser.hpp`
- `WhatSonNoteHeaderStore.cpp`
- `WhatSonNoteHeaderStore.hpp`

## Current Focus Areas
- `src/app/models/editor/persistence/ContentsEditorPersistenceController` now owns the editor-side buffered persistence drain clock and best-effort
  lifecycle flush requests. This `file/note` directory only owns the downstream note-package management queue once a
  snapshot is already selected for async persistence.
- `ContentsNoteManagementCoordinator` now owns editor-adjacent note-management orchestration:
  - direct `.wsnote` persistence serialization
  - header-only `openCount` / `lastOpenedAt` updates
  - tracked-stat refresh follow-up
  - post-persist metadata resync back into the bound content view-model
- Shared derived-statistic helpers now live under `src/app/models/file/statistic/WhatSonNoteFileStatSupport.*` rather than in
  this note-package directory.
- `.wsnhead` now carries a dedicated `fileStat` block for numeric detail-panel metadata.
- Note creation, note update, and editor note selection all participate in keeping that block
  synchronized with the current body/header state.
- `WhatSonIiXmlDocumentSupport` owns the local iiXml document-tree boundary for note package reads, including preamble
  stripping, tag lookup, descendant collection, text extraction, and attribute extraction.
- `WhatSonNoteHeaderParser` now uses that shared support layer for `.wsnhead` tag and attribute extraction, keeping XML
  parsing aligned with the planned RAW note -> HTML block pipeline instead of relying on ad-hoc text scans.
- `WhatSonLocalNoteFileStore` now uses the same support layer for `.wsnbody` package reads when locating `<body>` and
  first-resource thumbnail metadata.
- Editor note selection now uses a header-only `openCount` rewrite path that also stamps `.wsnhead lastOpenedAt`, so
  switching notes no longer forces a hub-wide `.wsnbody` backlink rescan and inactivity sensors can read the true
  last-open time directly from RAW header state.
- The `fileStat` schema is tracked as a documented package contract; this repository no longer maintains a dedicated
  scripted test for it.
- Empty-note body parsing now strips formatting-only `<body>` indentation so a newly created note opens on the first
  editable line instead of showing a phantom leading blank line.
- Body-save normalization now also owns inline hashtag promotion:
  - editor-visible `#label` source persists into `.wsnbody` as `<tag>label</tag>`
  - the same save transaction unions that tag into `.wsnhead`
  - new tags are inserted into `Tags.wstags` so the tags hierarchy can reload them as first-class entries
- Body-save normalization now also preserves proprietary inline formatting across paragraph boundaries by reopening any
  still-active style tags at the next serialized `<paragraph>`.
- RAW resource import now also shares one static note-body tag generator:
  drag/drop and clipboard-image insertion both normalize imported metadata through
  `WhatSonNoteBodyResourceTagGenerator.*` before mutating `.wsnbody`.
- Automated C++ regression coverage now lives in `test/cpp/suites/*.cpp`, locking canonical
  resource-tag generation through `WhatSonNoteBodyResourceTagGenerator` / `ContentsResourceTagTextGenerator` and
  raw-folder-block inspection through `WhatSonNoteFolderSemantics`.
- `.wsnbody` semantic tag classification is now owned by `WhatSonNoteBodySemanticTagSupport.*` so the note-body save
  path and the editor HTML read paths resolve legacy body tags through the same registry.
- RAW note hyperlinks are now centralized in `WhatSonNoteBodyWebLinkSupport.*`:
  - typed/pasted committed URLs can be promoted into canonical `<weblink href="...">...</weblink>` RAW spans
  - `.wsnbody` body serialization and RichText projection reuse the same helper, so saved/body HTML and live editor
    preview do not disagree about which href should open externally
- `fileStat.modifiedCount` is now the local commit counter for note package history.
  - whenever it advances, `.wsnversion` appends a snapshot with the matching `commitModifiedCount`
  - snapshot/diff persistence is delegated to `src/app/models/file/diff/WhatSonLocalNoteVersionStore.*`


## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/file/note`` (`docs/src/app/models/file/note/README.md`)
- 위치: `docs/src/app/models/file/note`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
