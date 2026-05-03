# `src/app/models/file/note/WhatSonIiXmlDocumentSupport.hpp`

## Responsibility

Declares the note-package iiXml support boundary shared by `.wsnhead` and `.wsnbody` readers.

## Contract

- Owns UTF-8 view conversion, XML entity decoding, XML preamble stripping, tag-name comparison, field-name comparison,
  descendant lookup, text extraction, attribute extraction, and document parsing.
- Keeps note package parsers from duplicating iiXml adapter code in each consuming file.
- Exposes only read-side helpers. It does not mutate note source, normalize editor body contents, or decide note
  persistence policy.
