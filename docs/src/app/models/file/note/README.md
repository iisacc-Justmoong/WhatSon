# `src/app/models/file/note`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/note`
- Child directories: 8
- Child files: 0 at the source root

## Child Directories
- `body` - `.wsnbody` persistence, body tag helpers, body-local formatting helpers.
- `folder` - note-folder binding repository, service, and raw folder block semantics.
- `header` - `.wsnhead` creation, parsing, storage, and header-local metadata helpers.
- `hub` - mounted-hub note creation, deletion, folder clearing, and mutation support.
- `local` - local note package document and file-store API.
- `package` - note package bootstrap and initial body creation.
- `session` - content-panel note management orchestration.
- `support` - note-domain support helpers shared by the package readers.

## Child Files
- No C++ source or header files stay directly under `src/app/models/file/note`.

## Current Focus Areas
- This `file/note` directory owns the note-package management queue, concrete `.wsnote/.wsnbody` mutation, and
  follow-up metadata/stat refresh work for note body saves. Concrete files are classified into responsibility
  subdirectories instead of remaining in the shard root.
- `body` owns body serialization and body-local RAW source helpers.
- `header` owns `.wsnhead` schema creation, parsing, and storage.
- `local` owns the concrete `.wsnote` package document and file-store boundary.
- `hub` owns mounted-hub note mutation services.
- `folder` owns note-folder binding persistence and semantics.
- `package` owns note package bootstrap helpers.
- `session` owns content-panel note management orchestration.
- `support` owns note-domain support code shared across package readers.
- `ContentsNoteManagementCoordinator` owns note-management orchestration:
  - direct `.wsnote` persistence serialization
  - lazy selected-note body reads from the resolved `.wsnote` package
  - header-only `openCount` / `lastOpenedAt` updates
  - tracked-stat refresh follow-up
  - post-persist metadata resync back into the bound content view-model
- Its worker-lane completions are delivered through the application object and guarded back to the coordinator, so late
  selected-note body-read results are dropped safely after coordinator teardown.
- Shared derived-statistic helpers now live under `src/app/models/file/statistic/WhatSonNoteFileStatSupport.*` rather than in
  this note-package directory.
- `.wsnhead` now carries a dedicated `fileStat` block for numeric detail-panel metadata.
- Note creation, note update, and note selection all participate in keeping that block
  synchronized with the current body/header state.
- `WhatSonIiXmlDocumentSupport` owns the local iiXml document-tree boundary for note package reads, including preamble
  stripping, tag lookup, descendant collection, text extraction, and attribute extraction.
- `WhatSonNoteHeaderParser` now uses that shared support layer for `.wsnhead` tag and attribute extraction, keeping XML
  parsing aligned with the planned RAW note -> HTML block pipeline instead of relying on ad-hoc text scans.
- `WhatSonLocalNoteFileStore` now uses the same support layer for `.wsnbody` package reads when locating `<body>` and
  first-resource thumbnail metadata.
- Note selection now uses a header-only `openCount` rewrite path that also stamps `.wsnhead lastOpenedAt`, so
  switching notes no longer forces a hub-wide `.wsnbody` backlink rescan and inactivity sensors can read the true
  last-open time directly from RAW header state.
- The `fileStat` schema is tracked as a documented package contract; this repository no longer maintains a dedicated
  scripted test for it.
- Empty-note body parsing now strips formatting-only `<body>` indentation so a newly created note opens on the first
  editable line instead of showing a phantom leading blank line.
- Body-save normalization also owns inline hashtag promotion:
  - visible `#label` source persists into `.wsnbody` as `<tag>label</tag>`
  - the same save transaction unions that tag into `.wsnhead`
  - new tags are inserted into `Tags.wstags` so the tags hierarchy can reload them as first-class entries
- Body-save normalization now also preserves proprietary inline formatting across paragraph boundaries by reopening any
  still-active style tags at the next serialized `<paragraph>`.
- RAW resource import uses one static note-body tag generator:
  imported resource metadata is normalized through `WhatSonNoteBodyResourceTagGenerator.*` before mutating `.wsnbody`.
- Automated C++ regression coverage now lives in `test/cpp/suites/*.cpp`, locking canonical resource-tag generation
  through `WhatSonNoteBodyResourceTagGenerator` and raw-folder-block inspection through `WhatSonNoteFolderSemantics`.
- `.wsnbody` semantic tag classification is owned by `WhatSonNoteBodySemanticTagSupport.*` so the note-body save path
  resolves legacy body tags through one registry.
- `.wsnbody` body-format blocks are owned by the note package layer: `resource` and `break` persist as direct `<body>`
  children when they appear as standalone source lines.
- RAW note hyperlinks are now centralized in `WhatSonNoteBodyWebLinkSupport.*`:
  - typed/pasted committed URLs can be promoted into canonical `<weblink href="...">...</weblink>` RAW spans
  - `.wsnbody` body serialization reuses the same helper so saved source stays canonical
- `fileStat.modifiedCount` is now the local commit counter for note package history.
  - changed body saves increment the counter in the same `.wsnhead` transaction that persists the RAW body and captures
    a `.wsnversion` diff
  - unchanged body snapshots and timestamp-only header rewrites must not inflate the counter
  - whenever it advances, `.wsnversion` appends a snapshot with the matching `commitModifiedCount`
  - snapshot/diff persistence is delegated to `src/app/models/file/diff/WhatSonLocalNoteVersionStore.*`
- The shared IO object layer has been removed. Note package CRUD now uses note-domain mutation support for direct Qt
  filesystem operations, including atomic `QSaveFile` UTF-8 overwrites.


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
- 역할: 이 파일은 note 파일 shard의 책임별 하위 디렉터리 구조, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
