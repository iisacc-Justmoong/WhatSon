# File Sync

This shard contains editor RAW push/pull ordering helpers and timestamp conflict support.

The active contents editor no longer connects to these helpers because the document session model was removed. Existing classes may remain for non-mounted tests or future replacement work, but no QML route currently binds them to `LV.TextEditor`.
