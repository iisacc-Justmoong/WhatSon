# `src/app/models/editor/parser/ContentsWsnBodyBlockParser.hpp`

## Responsibility
Declares the dedicated `.wsnbody` block parser used by the editor read path.

## Public Contract
- `ParseResult.correctedSourceText`: canonical RAW source suggested by `WhatSonStructuredTagLinter`.
- `ParseResult.renderedDocumentBlocks`: one ordered block stream for the structured editor host.
- `ParseResult.renderedAgendas` / `ParseResult.renderedCallouts`: compatibility projections sourced from the same parse
  pass, not separate backend reparses.
- `ParseResult.agendaParseVerification` / `calloutParseVerification` / `structuredParseVerification`: linter-backed
  verification payloads for the parsed RAW snapshot.
- `parse(sourceText)`: parses one authoritative RAW `.wsnbody` snapshot and returns a renderer-ready projection.

## Architectural Note
- This type lives under `editor/parser` because it is a read-side document parser for editor presentation, not a
  mutation-authoritative persistence model.
- The RAW `.wsnbody` file remains the source of truth; this parser only derives block coordinates and render payloads
  from that snapshot.
