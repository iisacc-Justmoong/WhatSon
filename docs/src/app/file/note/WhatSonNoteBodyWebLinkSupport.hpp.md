# `src/app/file/note/WhatSonNoteBodyWebLinkSupport.hpp`

## Role
Declares the shared RAW-note weblink helper that keeps editor mutation, `.wsnbody` serialization, and RichText
projection aligned on one hyperlink contract.

## Exposed Contracts
- `isWebLinkTagName(...)` identifies canonical `<weblink ...>` elements.
- `containsDetectableWebLink(...)` reports whether a literal source segment already contains an auto-linkifiable URL.
- `canonicalStartTag(...)` and `canonicalStartTagFromRawToken(...)` normalize RAW source into
  `<weblink href="...">`.
- `activationUrlForHref(...)` upgrades scheme-less `www...` href values into externally openable URLs.
- `openingHtmlForHref(...)` and `openingHtmlFromRawToken(...)` project RAW link metadata into RichText `<a href=...>`
  anchors.
- `autoWrapDetectedWebLinks(...)` scans literal source text outside existing tags and wraps detected web URLs back into
  canonical RAW `<weblink>` tags.
