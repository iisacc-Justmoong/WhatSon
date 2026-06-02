# `src/app/models/clipboard/InAppClipboardManager.h`

`InAppClipboardManager` orchestrates clipboard and URL resource imports for the active hub.

It owns hub path configuration, current resource state delegation, package creation, metadata output, and error reporting.

The manager no longer coordinates editor document insertion. Its import results are package metadata only.
