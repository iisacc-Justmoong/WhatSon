# Clipboard Model

The clipboard model shard owns app clipboard resource state and resource-package import.

## Files

- `InAppClipboardStore.*`: stores the current in-app resource snapshot.
- `InAppClipboardManager.*`: imports clipboard or URL resources into `.wsresource` packages.
- `ClipboardResourceImport.*`: builds import descriptors from captured formats.
- `FiletypeCapture.*`: detects file type and MIME hints.

## Current Boundary

Clipboard import stops at persisted resource packages and resource list updates. The editor document paste bridge was deleted, so this shard must not insert RAW body tags, mutate active editor text, or expose editor item key-filter behavior.
